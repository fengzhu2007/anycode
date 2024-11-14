#ifndef ZOOM_LABEL_H
#define ZOOM_LABEL_H

#include <QLabel>
#include <QModelIndex>
namespace ady{
class ZoomLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ZoomLabel(QWidget* parent);
    void setZoom(int zoom);
signals:
    void selected(int zoom);


protected:
    void mousePressEvent(QMouseEvent *e) override;


};
}


#endif // ZOOM_LABEL_H
