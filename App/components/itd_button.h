#ifndef ITDBUTTON_H
#define ITDBUTTON_H
#include "global.h"
#include <QFrame>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QEnterEvent>
#endif
namespace ady{
    class ITDButtonPrivate;
    class ANYENGINE_EXPORT ITDButton : public QFrame
    {
        Q_OBJECT
    public:
        explicit ITDButton(QWidget* parent);
        ~ITDButton();
        void setIcon(const QString& src);
        void setText(const QString& title);
        void setDescription(const QString& description);
    signals:
        void clicked();
    protected:
        virtual void paintEvent(QPaintEvent *e) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        virtual void enterEvent(QEnterEvent *event) override;
#else
        virtual void enterEvent(QEvent *event) override;
#endif
        virtual void leaveEvent(QEvent *event) override;
        virtual void mousePressEvent(QMouseEvent *event) override;
        virtual void mouseMoveEvent(QMouseEvent *event) override;
        virtual void mouseReleaseEvent(QMouseEvent *event) override;

    private:
        ITDButtonPrivate* d;

    };
}
#endif // ITDBUTTON_H
