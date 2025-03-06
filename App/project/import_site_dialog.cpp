#include "import_site_dialog.h"
#include "ui_import_site_dialog.h"
#include "storage/project_storage.h"
#include "storage/site_storage.h"
#include <QAbstractItemModel>

namespace ady{

class ImportSiteModelItem{
public:
    enum Type{
        Root,
        Project,
        Site
    };
    ImportSiteModelItem(){
        this->type = Root;
        this->parent = nullptr;
    }

    ImportSiteModelItem(Type type,long long id,QString name,QString addon,int status,ImportSiteModelItem* parent){
        this->type = type;
        this->id = id;
        this->name = name;
        this->addonType = addon;
        this->status = status;
        this->parent = parent;
    }


    ~ImportSiteModelItem(){
        qDeleteAll(children);
    }

    ImportSiteModelItem* childAt(int row){
        return children.at(row);
    }

    int row(){
        return parent->children.indexOf(this);
    }

public:
    Type type;
    long long id;
    QString name;
    QString addonType;
    int status;
    ImportSiteModelItem* parent;
    QList<ImportSiteModelItem*> children;
};


class ImportSiteModelPrivate{
public:
    ImportSiteModelItem* root;
};

class ImportSiteModel : public QAbstractItemModel{
private:
    ImportSiteModelPrivate* d;

public:
    enum Column {
        Name=0,
        Type,
        Status,
        Max
    };
    explicit ImportSiteModel(QObject* parent){
        d = new ImportSiteModelPrivate;
        d->root = new ImportSiteModelItem();
        //list all project
        auto projectlist = ProjectStorage().all();
        auto sitelist = SiteStorage().all();

        for(auto proj:projectlist){
            ImportSiteModelItem* pj = nullptr;
            for(auto site:sitelist){
                if(proj.id==site.pid){
                    if(pj==nullptr){
                        pj = new ImportSiteModelItem{ImportSiteModelItem::Project,proj.id,proj.name,"",0,d->root};
                        d->root->children.append(pj);
                    }
                    auto ss = new ImportSiteModelItem{ImportSiteModelItem::Site,site.id,site.name,site.type,site.status,pj};
                    pj->children.append(ss);
                }
            }
        }


    }

    ~ImportSiteModel(){
        delete d;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override{
        if(role==Qt::DisplayRole){
            if(section==Name){
                return tr("Name");
            }else if(section==Type){
                return tr("Type");
            }else if(section==Status){
                return tr("Status");
            }
        }
        return {};
    }

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override{
        if (!hasIndex(row, column, parent))
            return QModelIndex();

        ImportSiteModelItem *parentItem;

        if (!parent.isValid())
            parentItem = d->root;
        else
            parentItem = static_cast<ImportSiteModelItem*>(parent.internalPointer());

        ImportSiteModelItem *childItem = parentItem->childAt(row);
        if (childItem)
            return createIndex(row, column, childItem);
        else
            return QModelIndex();
    }
    QModelIndex parent(const QModelIndex &index) const override{
        if (!index.isValid())
            return QModelIndex();

        ImportSiteModelItem *childItem = static_cast<ImportSiteModelItem*>(index.internalPointer());
        if(childItem==d->root || childItem==nullptr){
            return QModelIndex();
        }
        //qDebug()<<"item:"<<childItem<<index.row();

        ImportSiteModelItem *parentItem = childItem->parent;
        if (parentItem == d->root || parentItem==nullptr)
            return QModelIndex();
        return createIndex(parentItem->row(), 0, parentItem);
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override{
        if (!parent.isValid())
            return d->root->children.length();

        ImportSiteModelItem* parentItem = static_cast<ImportSiteModelItem*>(parent.internalPointer());
        if(parentItem!=nullptr){
            return parentItem->children.length();
        }else{
            return 0;
        }
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override{
        return Max;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override{
        if (!index.isValid())
            return QVariant();
        if(role == Qt::DisplayRole){
            int col = index.column();
            ImportSiteModelItem* item = static_cast<ImportSiteModelItem*>(index.internalPointer());
             if(col==Name){
                return item->name;
             }else if(col==Type){
                 return item->addonType;
             }else if(col==Status){
                 if(item->type==ImportSiteModelItem::Site){
                     return item->status==1?tr("OK"):tr("Disabled");
                 }
             }
        }
        return {};
    }

};




ImportSiteDialog::ImportSiteDialog(QWidget *parent)
    : wDialog(parent)
    , ui(new Ui::ImportSiteDialog)
{
    ui->setupUi(this);
    this->resetupUi();
    setAttribute(Qt::WA_DeleteOnClose);

    ui->treeView->setModel(new ImportSiteModel(ui->treeView));
    ui->treeView->expandAll();

    connect(ui->treeView,&QTreeView::doubleClicked,this,&ImportSiteDialog::onSelected);

    ui->treeView->setColumnWidth(ImportSiteModel::Name,180);
}

ImportSiteDialog::~ImportSiteDialog()
{
    delete ui;
}

void ImportSiteDialog::onSelected(const QModelIndex& index){
    ImportSiteModelItem* item = static_cast<ImportSiteModelItem*>(index.internalPointer());
    if(item->type==ImportSiteModelItem::Site){
        emit selected(item->id);
    }
}

ImportSiteDialog* ImportSiteDialog::open(QWidget* parent){
    auto win = new ImportSiteDialog(parent);
    win->setModal(true);
    win->show();
    return win;
}

}
