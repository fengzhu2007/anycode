#include "grid_view.h"

namespace ady{

GridView::GridView(QWidget* parent)
    :QListView(parent){


}

void GridView::addMimeType(const QString& mimeType)
{
    this->supportMimeTypes<<mimeType;
}

void GridView::setMimeTypes(const QStringList& mimeTypes)
{
    this->supportMimeTypes = mimeTypes;
}

void GridView::setSupportDropFile(bool res)
{
    this->supportDropFile = res;
}


bool GridView::dropMimeData(const QMimeData* data)
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

void GridView::dropEvent(QDropEvent* event)
{
    if(this->dropMimeData(event->mimeData())){
        emit dropFinished(event->mimeData());
        event->accept();
    }else{
        event->ignore();
    }
}


void GridView::dragEnterEvent(QDragEnterEvent* event)
{
    if(this->dropMimeData(event->mimeData())){
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
    }else{
        event->ignore();
    }
}

void GridView::dragMoveEvent(QDragMoveEvent *event)
{
    if(this->dropMimeData(event->mimeData())){
        event->accept();
    }else{
        event->ignore();
    }
}

}
