#include "DirComboBox.h"
#include <QHeaderView>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QStylePainter>
namespace ady {

DirModel::DirModel(QObject* parent)
    :QDirModel(parent)
{

    this->setSorting(QDir::DirsFirst | QDir::Name);
    this->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
}




int  DirModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

DirTreeView::DirTreeView(QWidget* parent)
    :QTreeView(parent)
{

}


void DirTreeView::leaveEvent(QEvent *event)
{
    QTreeView::leaveEvent(event);
}

void DirTreeView::focusOutEvent(QFocusEvent *event)
{
    QTreeView::focusOutEvent(event);
}

 bool DirTreeView::event(QEvent *event)
 {
    switch(event->type()){
        case QEvent::Hide:
        case QEvent::MouseButtonPress:
            this->hide();
        break;

    }
     return QTreeView::event(event);
 }



DirComboBox::DirComboBox(QWidget* parent)
    :QComboBox(parent)
{
    m_popupView = new DirTreeView(this);
    m_popupView->setWindowFlags(Qt::Popup);
    m_popupView->hide();

    m_popupView->setHeaderHidden(true);
    QHeaderView* header = m_popupView->header();
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_popupView->setModel(new DirModel);


    connect(m_popupView,&QTreeView::activated,this,&DirComboBox::onActivated);
    connect(m_popupView,&QTreeView::clicked,this,&DirComboBox::onActivated);

}

void DirComboBox::setPath(const QString & path)
{
    this->setCurrentText(path);
    DirModel* model = (DirModel*)this->m_popupView->model();
    QModelIndex index = model->index(path);
    this->m_popupView->expand(index);
}

void DirComboBox::showPopup()
{
    QPoint pos = QWidget::mapToGlobal(this->pos());
    QRect rc = this->frameGeometry();
    int width = rc.width();
    int height = rc.height();
    QRect rc2(pos - QPoint(rc.x(),rc.y() - height),QSize(width,200));
    m_popupView->setGeometry(rc2);
    m_popupView->show();
}

void DirComboBox::hidePopup()
{
    QComboBox::hidePopup();
    if(m_popupView!=nullptr){
        m_popupView->hide();
    }
}

bool DirComboBox::eventFilter(QObject* object, QEvent* event)
{
        if ((event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick) && object == view()->viewport())
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QModelIndex index = view()->indexAt(mouseEvent->pos());
            if (!view()->visualRect(index).contains(mouseEvent->pos())){
                skipNextHide = true;
            }else{
                QDirModel* model = (QDirModel*)this->model();
                QFileInfo fi = model->fileInfo(index);
                m_selectedText = fi.filePath();
            }
        }
        return false;
}



void DirComboBox::onActivated(const QModelIndex &index)
{
    QDirModel* model = (QDirModel*)m_popupView->model();
    QFileInfo fi = model->fileInfo(index);
    QIcon icon = model->fileIcon(index);
    this->setWindowIcon(icon);
    this->setCurrentText(fi.filePath());
    this->hidePopup();
    emit currentTextChanged(this->currentText());
}

void DirComboBox::onPathChanged(QString & path)
{
    this->setPath(path);
    //emit currentTextChanged(this->currentText());
}

}
