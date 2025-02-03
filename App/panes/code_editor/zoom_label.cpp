#include "zoom_label.h"
#include "core/theme.h"
#include <QStringListModel>
#include <QMouseEvent>
#include <QMenu>
#include <QDebug>
namespace ady{

static int zooms[] = {20,50,70,100,150,200,400};

ZoomLabel::ZoomLabel(QWidget* parent):QLabel(parent) {
    auto theme = Theme::getInstance();
    auto secondaryColor = theme->secondaryColor().name(QColor::HexRgb);
    this->setStyleSheet("QLabel{padding:0 8px}"
                                      "QLabel::hover{background:"+secondaryColor+";border:1px solid #007acc;}");
}



void ZoomLabel::setZoom(int zoom){
    this->setText(tr("%1%").arg(zoom));
}


void ZoomLabel::mousePressEvent(QMouseEvent *e){
    QLabel::mousePressEvent(e);
    QMenu contextMenu(this);
    int length = sizeof(zooms) / sizeof(zooms[0]);
    for(int i=0;i<length;i++){
        int zoom = zooms[i];
        contextMenu.addAction(tr("%1%").arg(zooms[i]),[this,zoom]{
            this->setZoom(zoom);
            emit selected(zoom);
        });
    }
    contextMenu.exec(QCursor::pos());
}


}
