#ifndef PASSWORD_EDIT_H
#define PASSWORD_EDIT_H
#include "global.h"
#include <QLineEdit>

namespace ady{
class PasswordEditPrivate;
class ANYENGINE_EXPORT PasswordEdit : public QLineEdit
{
    Q_OBJECT
public:
    enum State{
        Visible,
        Invisible
    };
    explicit PasswordEdit(QWidget* parent=nullptr);
    ~PasswordEdit();
    void setEchoMode(EchoMode mode);
    State state();
    void setState(bool state);
    void setState(State state);

signals:
    void visibleChanged(bool);
protected:
    virtual void paintEvent(QPaintEvent *e) override;

public slots:
    void onVisible();

private:
    PasswordEditPrivate* d;

};
}

#endif // PASSWORD_EDIT_H
