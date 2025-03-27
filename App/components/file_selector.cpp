#include "file_selector.h"
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>

namespace ady{
class FileSelectorPrivate{
public:
    QLineEdit* edit;
    QPushButton* button;
    QString filter;
    QString dir;
};

FileSelector::FileSelector(QWidget *parent,const QString& filter)
    : QWidget{parent}
{
    this->setStyleSheet("QPushButton{height:24px;padding:0;}");
    d = new FileSelectorPrivate;
    d->filter = filter;
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

    connect(d->button,&QPushButton::clicked,this,&FileSelector::onSelectFile);
}


FileSelector::~FileSelector(){
    delete d;
}

void FileSelector::setEnable(bool enable){
    d->button->setEnabled(enable);
    d->edit->setEnabled(enable);
}

void FileSelector::setFilter(const QString& filter){
    d->filter = filter;
}

QString& FileSelector::filter(){
    return d->filter;
}

void FileSelector::setDir(const QString& dir){
    d->dir = dir;
}

QString& FileSelector::dir(){
    return d->dir;
}

void FileSelector::setText(const QString& text){
    d->edit->setText(text);
}

QString FileSelector::text() const{
    return d->edit->text();
}

void FileSelector::onSelectFile(){
    auto filePath = QFileDialog::getOpenFileName(this,tr("Select File"),d->dir,d->filter);
    if(!filePath.isEmpty()){
        d->edit->setText(filePath);
        emit fileChanged(filePath);
    }
}



}
