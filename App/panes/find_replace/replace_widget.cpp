#include "replace_widget.h"
#include "ui_replace_widget.h"
#include "find_replace_dialog.h"
#include "search_scope_model.h"
#include "components/list_item_delegate.h"

#include <QFileDialog>
#include <QDebug>
namespace ady{
class ReplaceWidgetPrivate{
public:
    QString find;
    QString replace;
};

ReplaceWidget::ReplaceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReplaceWidget)
{
    ui->setupUi(this);
    d = new ReplaceWidgetPrivate;

    connect(ui->findPrevious,&QPushButton::clicked,this,&ReplaceWidget::onClicked);
    connect(ui->findNext,&QPushButton::clicked,this,&ReplaceWidget::onClicked);
    connect(ui->replaceOne,&QPushButton::clicked,this,&ReplaceWidget::onClicked);
    connect(ui->replaceAll,&QPushButton::clicked,this,&ReplaceWidget::onClicked);
    connect(ui->selectFolder,&QPushButton::clicked,this,&ReplaceWidget::onSelectFolder);
    connect(ui->searchScope,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&ReplaceWidget::onSearchScopeChanged);

    auto dialog = static_cast<FindReplaceDialog*>(parent->parentWidget());
    ui->find->setModel(dialog->findModel());
    ui->searchScope->setModel(dialog->searchScodeModel());
    ui->filePattern->setModel(dialog->filePatternModel());
    ui->exclusion->setModel(dialog->exclusionModel());
    ui->filePattern->setCurrentIndex(0);
    ui->exclusion->setCurrentIndex(0);

    ui->find->setItemDelegate(new ListItemDelegate(ui->find));
    ui->replace->setItemDelegate(new ListItemDelegate(ui->filePattern));
    ui->searchScope->setItemDelegate(new ListItemDelegate(ui->searchScope));
    ui->exclusion->setItemDelegate(new ListItemDelegate(ui->exclusion));
    ui->filePattern->setItemDelegate(new ListItemDelegate(ui->filePattern));

}

ReplaceWidget::~ReplaceWidget()
{
    delete d;
    delete ui;
}

void ReplaceWidget::clear(){
    d->find.clear();
    d->replace.clear();
}

void ReplaceWidget::setFindText(int i){
    ui->find->setCurrentIndex(i);
}

void ReplaceWidget::setSearchScope(int i){
    ui->searchScope->setCurrentIndex(i);
}

void ReplaceWidget::onClicked(){
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
    const QString before = ui->find->currentText();
    const QString after = ui->replace->currentText();
    if(before==after){
        return ;
    }
    bool hightlight = false;
    if(d->find!=before){
        hightlight = true;
        d->find = before;
    }
    if(sender==ui->findPrevious){
        bit |= 0x01;
        dialog->search(before,bit,hightlight);
    }else if(sender==ui->findNext){
        dialog->search(before,bit,hightlight);
    }else if(sender==ui->replaceOne){
        dialog->replace(before,after,bit,hightlight);
    }else if(sender==ui->replaceAll){
        dialog->replaceAll(before,after,ui->searchScope->currentText(),bit,ui->filePattern->currentText(),ui->exclusion->currentText());
        dialog->close();
    }else{
        return ;
    }
}

void ReplaceWidget::onSelectFolder(){
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
        list.insert(1,QPair{dir,dir});
        //list<<QPair{dir,dir};
        model->setDataSource(list);
        ui->searchScope->setCurrentIndex(1);
    }
}

void ReplaceWidget::onSearchScopeChanged(int i){
    if(i==0){
        //current document
        ui->filePattern->setEnabled(false);
        ui->exclusion->setEnabled(false);
    }else{
        ui->filePattern->setEnabled(true);
        ui->exclusion->setEnabled(true);
    }
}

void ReplaceWidget::showEvent(QShowEvent* e){
    QWidget::showEvent(e);
    ui->find->setFocus();
    ui->findNext->setDefault(true);
}

}
