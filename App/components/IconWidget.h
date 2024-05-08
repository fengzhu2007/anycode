#ifndef ICONWIDGET_H
#define ICONWIDGET_H
#include "global.h"
#include <QLabel>
namespace ady {
    class IconWidget : public QLabel
    {
        Q_OBJECT
    public:
        IconWidget(QWidget* parent);
        void setResources(QString normalRes,QString hoverRes);
    protected:
        virtual void enterEvent(QEvent *event) override;
        virtual void leaveEvent(QEvent *event) override;

        virtual void mousePressEvent(QMouseEvent *event) override;
        virtual void mouseReleaseEvent(QMouseEvent *event) override;

    signals:
        void clicked();

    private:
        QString m_normalRes;
        QString m_hoverRes;
        bool m_pressed;

    };
}
#endif // ICONWIDGET_H
