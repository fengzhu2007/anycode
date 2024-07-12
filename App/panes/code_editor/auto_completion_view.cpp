#include "auto_completion_view.h"
#include "language.h"
#include <QFileIconProvider>
#include <QDebug>

namespace ady{

AutoCompletionModel* AutoCompletionModel::instance = nullptr;

class AutoCompletionModelPrivate{

public:
    QVector<AutoCompletionItem> list;
    //QFileIconProvider* provider;
    QPixmap keywordIcon;
    QPixmap classIcon;
    QPixmap functionIcon;
    QPixmap variantIcon;
    QPixmap constantIcon;
    QPixmap fieldIcon;
};

AutoCompletionModel::AutoCompletionModel()
    :QAbstractItemModel(){

    d = new AutoCompletionModelPrivate;
    d->keywordIcon = QIcon(":/Resource/icons/IntelliSenseKeyword_16x.svg").pixmap(14,14);
    d->classIcon = QIcon(":/Resource/icons/Class_16x.svg").pixmap(14,14);
    d->functionIcon = QIcon(":/Resource/icons/Method_16x.svg").pixmap(14,14);
    d->variantIcon = QIcon(":/Resource/icons/LocalVariable_16x.svg").pixmap(14,14);
    d->constantIcon = QIcon(":/Resource/icons/Constant_16x.svg").pixmap(14,14);
    d->fieldIcon = QIcon(":/Resource/icons/Field_16x.svg").pixmap(14,14);
    //d->provider = new QFileIconProvider;
}

AutoCompletionModel::~AutoCompletionModel(){
    //delete d->provider;
    delete d;
}

QModelIndex AutoCompletionModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    return createIndex(row, column);
}

QModelIndex AutoCompletionModel::parent(const QModelIndex &child) const{
     return QModelIndex();
}

QVariant AutoCompletionModel::data(const QModelIndex &index, int role) const{
     if(!index.isValid()){
        return QVariant();
     }else if(role==Qt::DisplayRole){
        return d->list.at(index.row()).name;
     }else if(role==Qt::DecorationRole){
        auto one = d->list.at(index.row());
        if(one.type==AutoCompletionItem::Keyword){
            return d->keywordIcon;
        }else if(one.type==AutoCompletionItem::Class){
            return d->classIcon;
        }else if(one.type==AutoCompletionItem::Function){
            return d->functionIcon;
        }else if(one.type==AutoCompletionItem::Variant){
            return d->variantIcon;
        }else if(one.type==AutoCompletionItem::Constant){
            return d->constantIcon;
        }else if(one.type==AutoCompletionItem::Field){
            return d->fieldIcon;
        }else{
            return QVariant();
        }
     }else{
        return QVariant();
     }

}

int AutoCompletionModel::rowCount(const QModelIndex &parent) const{
    return d->list.size();
}

int AutoCompletionModel::columnCount(const QModelIndex &parent) const  {
    return parent.isValid() ? 0 : 1;
}

void AutoCompletionModel::setDataSource(QVector<AutoCompletionItem> list){
    d->list = list;
}

void AutoCompletionModel::initLanguage(Language* language){
    beginResetModel();
    d->list.clear();
    if(language!=nullptr){
        {
            auto list = language->keywords();
            foreach(auto one,list){
                d->list.append({AutoCompletionItem::Keyword,one});
            }
        }
        {
            auto list = language->classes();
            foreach(auto one,list){
                d->list.append({AutoCompletionItem::Class,one});
            }
        }
        {
            auto list = language->functions();
            foreach(auto one,list){
                d->list.append({AutoCompletionItem::Function,one});
            }
        }
        {
            auto list = language->variants();
            foreach(auto one,list){
                d->list.append({AutoCompletionItem::Variant,one});
            }
        }
        {
            auto list = language->constants();
            foreach(auto one,list){
                d->list.append({AutoCompletionItem::Constant,one});
            }
        }
        {
            auto list = language->fields();
            foreach(auto one,list){
                d->list.append({AutoCompletionItem::Field,one});
            }
        }
    }
    endResetModel();

}


AutoCompletionModel* AutoCompletionModel::getInstance(){
    return instance;
}

void AutoCompletionModel::init(){
    if(instance==nullptr){
        instance = new AutoCompletionModel;
    }
}
void AutoCompletionModel::destory(){
    if(instance==nullptr){
        delete instance;
        instance = nullptr;
    }
}

/*************AutoCompletionView start****************/


class AutoCompletionViewPrivate{
public:
    AutoCompletionModel* model;

};

AutoCompletionView::AutoCompletionView(QWidget* parent)
    :QListView(parent)
{
    d = new AutoCompletionViewPrivate;
    d->model = nullptr;
}


AutoCompletionView::~AutoCompletionView(){
    delete d;
}

AutoCompletionModel* AutoCompletionView::model(){
    return d->model;
}

}
