#ifndef COLOR_PICKER_H
#define COLOR_PICKER_H


#include <QWidget>
#include <QMouseEvent>
#include <QPixmap>
#include <QPainter>
namespace ady{
class ColorPickerPrivate;
class ColorPicker : public QWidget
{
    Q_OBJECT
public:
    explicit ColorPicker(QWidget *parent = nullptr);
    ~ColorPicker();
signals:
    void colorPicked(const QColor &color);
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QColor grabScreenColor(const QPoint &pos);


private:
    ColorPickerPrivate* d;
};
}


#endif // COLOR_PICKER_H
