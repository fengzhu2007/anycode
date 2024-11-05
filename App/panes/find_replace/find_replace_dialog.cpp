#include "find_replace_dialog.h"
#include "ui_find_replace_dialog.h"
#include "find_widget.h"
#include "replace_widget.h"
#include "search_scope_model.h"

#include <QDebug>
namespace ady{

FindReplaceDialog* FindReplaceDialog::instance = nullptr;

class FindReplaceDialogPrivate{
public:
    ReplaceWidget* replace;
    FindWidget* find;
    QStringListModel* findModel;
    QStringListModel* replaceModel;
    QStringListModel* filePatternModel;
    QStringListModel* exclusionModel;
    SearchScopeModel* searchScodeModel;
};

FindReplaceDialog::FindReplaceDialog(QWidget *parent) :
    wDialog(parent),
    ui(new Ui::FindReplaceDialog)
{
    this->setStyleSheet("QComboBox{height:20px}"
                        "QTabWidget{border:0;}"
                        "QTabWidget::pane{border:0;border-top:1px solid #ccc;top:-1px}"
                        "QTabWidget::tab-bar{left:10px}"
                        "QTabBar::tab{border:0;background-color:transparent;padding:2px 8px;height:20px;}"
                        "QTabBar::tab:selected{border-bottom:2px solid #007acc}");
    ui->setupUi(this);
    d = new FindReplaceDialogPrivate;

    d->findModel = new QStringListModel(this);
    d->replaceModel = new QStringListModel(this);
    d->filePatternModel = new QStringListModel(this);
    d->exclusionModel = new QStringListModel(this);
    d->searchScodeModel = new SearchScopeModel(this);


    d->find = new FindWidget(ui->tabWidget);
    d->replace = new ReplaceWidget(ui->tabWidget);
    ui->tabWidget->addTab(d->find,tr("Find"));
    ui->tabWidget->addTab(d->replace,tr("Replace"));
    this->resetupUi();

    auto list = {QPair<QString,QString>(QString::fromUtf8(":"),tr("Current Document")),QPair<QString,QString>(QString::fromUtf8("::"),tr("Opend Documents"))};
    d->searchScodeModel->setDataSource(list);

    d->filePatternModel->setStringList({QString::fromUtf8("*.*")});
    d->exclusionModel->setStringList({QString::fromUtf8("*.git*,*.cvs*,*.svn*")});


    //this->setFindText("test");
    //this->setSearchScope("D:/wamp/www/test/phpcms/modules/search");

}

FindReplaceDialog::~FindReplaceDialog()
{
    instance = nullptr;
    delete ui;
    delete d;
}
void FindReplaceDialog::hideEvent(QHideEvent *event){
    emit cancelSearch();
    d->find->clear();
    d->replace->clear();
    wDialog::hideEvent(event);
}

QStringListModel* FindReplaceDialog::findModel(){
    return d->findModel;
}

QStringListModel* FindReplaceDialog::replaceModel(){
    return d->replaceModel;
}

QStringListModel* FindReplaceDialog::filePatternModel(){
    return d->filePatternModel;
}

QStringListModel* FindReplaceDialog::exclusionModel(){
    return d->exclusionModel;
}

SearchScopeModel* FindReplaceDialog::searchScodeModel(){
    return d->searchScodeModel;
}

void FindReplaceDialog::addSearch(const QString& text){
    if(!text.isEmpty()){
        QStringList list = d->findModel->stringList();
        list.removeOne(text);
        list.insert(0,text);
        d->findModel->setStringList(list);
    }
}

void FindReplaceDialog::addReplace(const QString& before,const QString& after){
    if(!before.isEmpty()){
        QStringList list = d->findModel->stringList();
        list.removeOne(before);
        list.insert(0,before);
        d->findModel->setStringList(list);
    }
    if(!after.isEmpty()){
        QStringList list = d->replaceModel->stringList();
        list.removeOne(after);
        list.insert(0,after);
        d->replaceModel->setStringList(list);
    }
}


void FindReplaceDialog::setFindText(const QString& text){
    auto list = d->findModel->stringList();
    list.removeOne(text);
    list.insert(0,text);
    d->findModel->setStringList(list);

    d->find->setFindText(0);
    d->replace->setFindText(0);
}
void FindReplaceDialog::setSearchScope(const QString& folder){
    if(folder==QString::fromUtf8(":")){
        d->find->setSearchScope(0);
        d->replace->setSearchScope(0);
        return ;
    }else  if(folder==QString::fromUtf8("::")){
        d->find->setSearchScope(1);
        d->replace->setSearchScope(1);
        return ;
    }
    auto list = d->searchScodeModel->toList();
    for(auto one:list){
        if(one.first==folder){
            list.removeOne(one);
            break;
        }
    }
    list<<QPair{folder,folder};
    //list.insert(1,QPair{folder,folder});
    d->searchScodeModel->setDataSource(list);
    d->find->setSearchScope(2);
    d->replace->setSearchScope(2);
}

void FindReplaceDialog::setMode(int mode){
    if(mode==0){
        //find
        ui->tabWidget->setCurrentIndex(0);
    }else{
        //replace
        ui->tabWidget->setCurrentIndex(1);
    }
}

FindReplaceDialog* FindReplaceDialog::getInstance(){
    return instance;
}

FindReplaceDialog* FindReplaceDialog::open(QWidget* parent){
    if(instance==nullptr){
        instance = new FindReplaceDialog(parent);
    }
    instance->show();
    return instance;
}


}
