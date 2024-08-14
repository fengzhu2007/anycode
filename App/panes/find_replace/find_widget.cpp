#include "find_widget.h"
#include "ui_find_widget.h"
#include "find_replace_dialog.h"
#include "search_scope_model.h"
#include <QFileDialog>
#include <QDebug>
namespace ady {
class FindWidgetPrivate{
public:
    QString find;
};

FindWidget::FindWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FindWidget)
{
    ui->setupUi(this);
    d = new FindWidgetPrivate;
    connect(ui->findPrevious,&QPushButton::clicked,this,&FindWidget::onClicked);
    connect(ui->findNext,&QPushButton::clicked,this,&FindWidget::onClicked);
    connect(ui->findAll,&QPushButton::clicked,this,&FindWidget::onClicked);
    connect(ui->selectFolder,&QPushButton::clicked,this,&FindWidget::onSelectFolder);
    connect(ui->searchScope,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&FindWidget::onSearchScopeChanged);
    //connect(ui->selectFolder,&QPushButton::clicked,this,&FindWidget::onSelectFolder)

    auto dialog = static_cast<FindReplaceDialog*>(parent->parentWidget());
    ui->find->setModel(dialog->findModel());
    ui->searchScope->setModel(dialog->searchScodeModel());
    ui->filePattern->setModel(dialog->filePatternModel());
    ui->exclusion->setModel(dialog->exclusionModel());
    ui->filePattern->setCurrentIndex(0);
    ui->exclusion->setCurrentIndex(0);
    ui->find->setFocus();

}

FindWidget::~FindWidget()
{
    delete d;
    delete ui;
}

void FindWidget::clear(){
    d->find.clear();
}

void FindWidget::setFindText(int i){
    ui->find->setCurrentIndex(i);
}

void FindWidget::setSearchScope(int i){
     ui->searchScope->setCurrentIndex(i);
}

/*
    FindBackward = 0x01,
    FindCaseSensitively = 0x02,
    FindWholeWords = 0x04,
    FindRegularExpression = 0x08,
    FindPreserveCase = 0x10
*/
void FindWidget::onClicked(){
    auto dialog = static_cast<FindReplaceDialog*>(parentWidget()->parentWidget()->parentWidget());
    auto sender = static_cast<QPushButton*>(this->sender());
    int bit = 0;
    if(ui->caseSensitive->isChecked()){
        bit |= 0x02;
    }
    if(ui->wholeWords->isChecked()){
        bit |= 0x04;
    }
    if(ui->regular->isChecked()){
        bit |= 0x08;
    }
    const QString text = ui->find->currentText();
    if(text.isEmpty()){
        return ;
    }
    bool hightlight = false;
    if(d->find!=text){
        hightlight = true;
        d->find = text;
        dialog->addSearch(text);
    }
    if(sender==ui->findPrevious){
        bit |= 0x01;
        dialog->search(text,bit,hightlight);
    }else if(sender==ui->findNext){
        dialog->search(text,bit,hightlight);
    }else if(sender==ui->findAll){
        dialog->searchAll(text,ui->searchScope->currentText(),bit,ui->filePattern->currentText(),ui->exclusion->currentText());
        dialog->close();
    }else{
        return ;
    }
}

void FindWidget::onSelectFolder(){
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Search Folder"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(!dir.isEmpty()){
        auto dialog = static_cast<FindReplaceDialog*>(parentWidget()->parentWidget()->parentWidget());
        auto model = dialog->searchScodeModel();
        auto list = model->toList();
        for(auto one:list){
            if(one.first==dir){
                list.removeOne(one);
                break;
            }
        }
        //insert front
        //list<<QPair{dir,dir};
        list.insert(1,QPair{dir,dir});
        model->setDataSource(list);
        ui->searchScope->setCurrentIndex(1);
    }

}

void FindWidget::onSearchScopeChanged(int i){
    if(i==0){
        //current document
        ui->filePattern->setEnabled(false);
        ui->exclusion->setEnabled(false);
    }else{
        ui->filePattern->setEnabled(true);
        ui->exclusion->setEnabled(true);
    }
}

void FindWidget::showEvent(QShowEvent* e){
    QWidget::showEvent(e);
    ui->find->setFocus();
}

}
