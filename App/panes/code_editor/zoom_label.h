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
    void initData();
public slots:
    void onIndexSelected(const QModelIndex& index);


protected:
    void mousePressEvent(QMouseEvent *e) override;

private:
         //SearchComboBoxPrivate* d;


};
}


#endif // ZOOM_LABEL_H
