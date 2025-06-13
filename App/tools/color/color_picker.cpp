#include "color_picker.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QDebug>
namespace ady{
class ColorPickerPrivate{
public:
    QPixmap screenshot;
    //QPixmap cursorPixmap;
};

ColorPicker::ColorPicker(QWidget *parent) : QWidget(parent)
{
    d = new ColorPickerPrivate;
    setMouseTracking(true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    QScreen *screen = QGuiApplication::primaryScreen();
    d->screenshot = screen->grabWindow(0);
    setFixedSize(d->screenshot.size());

    showFullScreen();
}

ColorPicker::~ColorPicker(){
    delete d;
    //qDebug()<<"ColorPicker close";
}

QColor ColorPicker::grabScreenColor(const QPoint &pos)
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QPixmap pixmap = screen->grabWindow(0, pos.x(), pos.y(), 1, 1);
    QImage image = pixmap.toImage();
    return image.pixel(0, 0);
}

void ColorPicker::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QColor color = grabScreenColor(event->globalPos());
        emit colorPicked(color);
        close();
    } else if (event->button() == Qt::RightButton) {
        //close();
        //refresh screen shot
        this->showMinimized();
        QScreen *screen = QGuiApplication::primaryScreen();
        d->screenshot = screen->grabWindow(0);
        showFullScreen();
        update();

    }
}

void ColorPicker::mouseMoveEvent(QMouseEvent *event)
{
    update();

}

void ColorPicker::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.drawPixmap(0, 0, d->screenshot);
    QPoint mousePos = QCursor::pos();
    QColor currentColor = grabScreenColor(mousePos);
    int zoomSize = 120;
    int realSize = 40;
    QPixmap zoomPixmap = d->screenshot.copy(mousePos.x() - realSize ,mousePos.y() - realSize / 2,realSize * 2, realSize);
    zoomPixmap = zoomPixmap.scaled(zoomSize ,zoomSize ,Qt::KeepAspectRatio,Qt::FastTransformation);
    painter.setPen(QPen(Qt::black, 1));
    auto x = mousePos.x() + 10;
    auto y = mousePos.y() + 20;

    if(x + zoomSize * 2 > rect().width()){
        //right
        x -= 20;
        x -= zoomSize * 2;
    }
    if(y + zoomSize > rect().height()){
        y -= 40;
        y -= zoomSize;
    }

    painter.drawPixmap(x, y,zoomSize * 2,zoomSize, zoomPixmap);
    painter.drawRect(x, y,zoomSize * 2,zoomSize);

    painter.setPen(QPen(Qt::red, 1));
    painter.drawLine(x + zoomSize, y , x + zoomSize, y + zoomSize);
    painter.drawLine(x, y + zoomSize / 2,x + zoomSize * 2,  y + zoomSize / 2);

    painter.setPen(Qt::white);
    painter.setBrush(QBrush(Qt::black));
    painter.drawRect(width() - 200, height() - 80, 190, 70);

    painter.drawText(width() - 170, height() - 60,
                     QString("RGB: %1, %2, %3")
                         .arg(currentColor.red())
                         .arg(currentColor.green())
                         .arg(currentColor.blue()));

    painter.drawText(width() - 170, height() - 40,
                     QString("HEX: %1").arg(currentColor.name()));

    painter.setBrush(QBrush(currentColor));
    painter.drawRect(width() - 200, height() - 80, 20, 20);
}

}
