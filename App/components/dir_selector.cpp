#include "dir_selector.h"
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
namespace ady{
class DirSelectorPrivate{
public:
    QLineEdit* edit;
    QPushButton* button;
    QString filter;
    QString dir;
};


DirSelector::DirSelector(QWidget *parent)
    : QWidget{parent}
{

    this->setStyleSheet("QPushButton{height:24px;padding:0;}");
    d = new DirSelectorPrivate;
    d->edit = new QLineEdit(this);
    d->button = new QPushButton(this);
    d->button->setIcon(QIcon(":/Resource/icons/SearchFolderClosed_16x.svg"));
    d->button->setAutoDefault(false);
    QHBoxLayout* layout = new QHBoxLayout(this);
    this->setLayout(layout);
    layout->addWidget(d->edit);
    layout->addWidget(d->button);
    layout->setMargin(0);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(2);

    connect(d->button,&QPushButton::clicked,this,&DirSelector::onSelectDir);

}




DirSelector::~DirSelector(){
    delete d;
}

void DirSelector::setEnable(bool enable){
    d->button->setEnabled(enable);
    d->edit->setEnabled(enable);
}



void DirSelector::setText(const QString& text){
    d->edit->setText(text);
}

QString DirSelector::text() const{
    return d->edit->text();
}

void DirSelector::onSelectDir(){
    auto folderPath = QFileDialog::getExistingDirectory(this,tr("Select Folder"));
    if(!folderPath.isEmpty()){
        d->edit->setText(folderPath);
        emit dirChanged(folderPath);
    }
}



}
