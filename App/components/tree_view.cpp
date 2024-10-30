#include "tree_view.h"
#include <QDebug>
namespace ady {
    TreeView::TreeView(QWidget* parent)
        :QTreeView(parent){


    }

    void TreeView::addMimeType(QString mimeType)
    {
        this->supportMimeTypes<<mimeType;
    }

    void TreeView::setMimeTypes(QStringList mimeTypes)
    {
        this->supportMimeTypes = mimeTypes;
    }

    void TreeView::setSupportDropFile(bool res)
    {
        this->supportDropFile = res;
    }


    bool TreeView::dropMimeData(const QMimeData* data)
    {
        if(this->supportMimeTypes.size()==0){
            if(supportDropFile){
                return data->hasUrls();
            }else{
                return false;
            }
        }
        int size = this->supportMimeTypes.size();
        for(int i=0;i<size;i++){
            if(data->hasFormat(this->supportMimeTypes[i])){
                return true;
            }
        }
        if(supportDropFile){
            return data->hasUrls();
        }else{
            return false;
        }
    }

    void TreeView::dropEvent(QDropEvent* event)
    {
        if(this->dropMimeData(event->mimeData())){
            emit dropFinished(event->mimeData());
            event->accept();
        }else{
            event->ignore();
        }
    }


    void TreeView::dragEnterEvent(QDragEnterEvent* event)
    {
        if(this->dropMimeData(event->mimeData())){
            event->setDropAction(Qt::CopyAction);
            event->acceptProposedAction();
        }else{
            event->ignore();
        }
    }

    void TreeView::dragMoveEvent(QDragMoveEvent *event)
    {
        if(this->dropMimeData(event->mimeData())){
            event->accept();
        }else{
            event->ignore();
        }
    }
}
