#include "file_selector.h"
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

// match fileName against QFileDialog style filter eg. "Images (*.png *.jpg);;All (*)"
static bool matchFilter(const QString& filter, const QString& fileName)
{
    if (filter.isEmpty()) {
        return true;
    }
    bool hasPattern = false;
    int pos = 0;
    while (true) {
        const int left = filter.indexOf(QLatin1Char('('), pos);
        if (left < 0) {
            break;
        }
        const int right = filter.indexOf(QLatin1Char(')'), left);
        if (right < 0) {
            break;
        }
        const QStringList patterns = filter.mid(left + 1, right - left - 1).split(QLatin1Char(' '));
        for (const QString& pattern : patterns) {
            if (pattern.isEmpty()) {
                continue;
            }
            hasPattern = true;
            if (pattern == QLatin1String("*")) {
                return true;
            }
            if (pattern.startsWith(QLatin1String("*."))) {
                if (fileName.endsWith(pattern.mid(1), Qt::CaseInsensitive)) {
                    return true;
                }
            } else if (fileName.compare(pattern, Qt::CaseInsensitive) == 0) {
                return true;
            }
        }
        pos = right + 1;
    }
    return !hasPattern;
}

static bool checkDroppedFile(const QMimeData* mimeData, const QString& filter, bool enabled, QString& filePath)
{
    if (!enabled || !mimeData->hasUrls()) {
        return false;
    }
    const QList<QUrl> urls = mimeData->urls();
    if (urls.isEmpty()) {
        return false;
    }
    filePath = urls.first().toLocalFile();
    if (filePath.isEmpty() || !QFileInfo(filePath).isFile()) {
        return false;
    }
    return matchFilter(filter, filePath);
}

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
    d->dir = "";
    connect(d->button,&QPushButton::clicked,this,&FileSelector::onSelectFile);
    this->setAcceptDrops(true);
    d->edit->setAcceptDrops(false);
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
    QString defaultDir;
    if (!d->edit->text().isEmpty()) {
        defaultDir = QFileInfo(d->edit->text()).path();
    } else if (!d->dir.isEmpty()) {
        defaultDir = d->dir;
    } else if (!s_lastDir.isEmpty()) {
        defaultDir = s_lastDir;
    } else {
        defaultDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    }
    auto filePath = QFileDialog::getOpenFileName(this,tr("Select File"),defaultDir,d->filter);
    if(!filePath.isEmpty()){
        d->edit->setText(filePath);
        s_lastDir = QFileInfo(filePath).path();
        emit fileChanged(filePath);
    }
}

void FileSelector::dragEnterEvent(QDragEnterEvent* event)
{
    QString filePath;
    if (checkDroppedFile(event->mimeData(), d->filter, d->edit->isEnabled(), filePath)) {
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void FileSelector::dragMoveEvent(QDragMoveEvent* event)
{
    QString filePath;
    if (checkDroppedFile(event->mimeData(), d->filter, d->edit->isEnabled(), filePath)) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void FileSelector::dropEvent(QDropEvent* event)
{
    QString filePath;
    if (checkDroppedFile(event->mimeData(), d->filter, d->edit->isEnabled(), filePath)) {
        d->edit->setText(filePath);
        s_lastDir = QFileInfo(filePath).path();
        emit fileChanged(filePath);
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}



}