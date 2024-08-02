#include "find_replace_dialog.h"
#include "ui_find_replace_dialog.h"
#include "find_widget.h"
#include "replace_widget.h"
#include <QDebug>
namespace ady{

FindReplaceDialog* FindReplaceDialog::instance = nullptr;

class FindReplaceDialogPrivate{
public:
    ReplaceWidget* replace;
    FindWidget* find;
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
    d->find = new FindWidget(ui->tabWidget);
    d->replace = new ReplaceWidget(ui->tabWidget);
    ui->tabWidget->addTab(d->find,tr("Find"));
    ui->tabWidget->addTab(d->replace,tr("Replace"));
    this->resetupUi();
}

FindReplaceDialog::~FindReplaceDialog()
{
    instance = nullptr;
    delete ui;
    delete d;
}
void FindReplaceDialog::hideEvent(QHideEvent *event){
    emit cancelSearch();
    wDialog::hideEvent(event);
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
