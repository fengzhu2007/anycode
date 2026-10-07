#include "dir_selector.h"
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFileInfo>
#include <QMimeData>
#include <QUrl>
namespace ady{

static QString s_lastDir;

static bool checkDroppedDir(const QMimeData* mimeData, bool enabled, QString& dirPath)
{
    if (!enabled || !mimeData->hasUrls()) {
        return false;
    }
    const QList<QUrl> urls = mimeData->urls();
    if (urls.isEmpty()) {
        return false;
    }
    dirPath = urls.first().toLocalFile();
    return !dirPath.isEmpty() && QFileInfo(dirPath).isDir();
}

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

    this->setAcceptDrops(true);
    d->edit->setAcceptDrops(false);
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
    QString defaultFolder;
    if (!d->edit->text().isEmpty()) {
        defaultFolder = d->edit->text();
    } else if (!s_lastDir.isEmpty()) {
        defaultFolder = s_lastDir;
    } else {
        defaultFolder = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    }
    auto folderPath = QFileDialog::getExistingDirectory(this,tr("Select Folder"),defaultFolder);
    if(!folderPath.isEmpty()){
        d->edit->setText(folderPath);
        s_lastDir = QFileInfo(folderPath).path();
        emit dirChanged(folderPath);
    }
}

void DirSelector::dragEnterEvent(QDragEnterEvent* event)
{
    QString dirPath;
    if (checkDroppedDir(event->mimeData(), d->edit->isEnabled(), dirPath)) {
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void DirSelector::dragMoveEvent(QDragMoveEvent* event)
{
    QString dirPath;
    if (checkDroppedDir(event->mimeData(), d->edit->isEnabled(), dirPath)) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void DirSelector::dropEvent(QDropEvent* event)
{
    QString dirPath;
    if (checkDroppedDir(event->mimeData(), d->edit->isEnabled(), dirPath)) {
        d->edit->setText(dirPath);
        s_lastDir = QFileInfo(dirPath).path();
        emit dirChanged(dirPath);
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}



}