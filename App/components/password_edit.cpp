#include "password_edit.h"
#include <QIcon>
#include <QAction>
#include <QStyleOption>
#include <QPainter>
namespace ady{

class PasswordEditPrivate{
public:
    QAction* visible;
    PasswordEdit::State state = PasswordEdit::Invisible;
    QIcon visibleIcon{":/Resource/icons/Visible_16x.svg"};
    QIcon invisibleIcon{":/Resource/icons/CloakOrHide_16x.svg"};

};

PasswordEdit::PasswordEdit(QWidget* parent):QLineEdit(parent) {

    setAttribute(Qt::WA_StyledBackground);
    d= new PasswordEditPrivate;
    d->visible = this->addAction(d->invisibleIcon,QLineEdit::TrailingPosition);
    connect(d->visible,&QAction::triggered,this,&PasswordEdit::onVisible);
    this->setEchoMode(QLineEdit::Password);
}

PasswordEdit::~PasswordEdit(){
    delete d;
}

void PasswordEdit::onVisible(){
    this->setState(d->state== PasswordEdit::Invisible? PasswordEdit::Visible: PasswordEdit::Invisible);
    emit visibleChanged(d->state==PasswordEdit::Visible);
}

void PasswordEdit::setEchoMode(EchoMode mode){
    Q_UNUSED(mode);
    //QLineEdit::setEchoMode(QLineEdit::PasswordEchoOnEdit);
    QLineEdit::setEchoMode(QLineEdit::Password);
    //Normal
}

PasswordEdit::State PasswordEdit::state(){
    return d->state;
}

void PasswordEdit::setState(bool state){
    setState(state?PasswordEdit::Visible:PasswordEdit::Invisible);
}

void PasswordEdit::setState(State state){
    d->state = state;
    if(d->state==PasswordEdit::Invisible){
        QLineEdit::setEchoMode(QLineEdit::Password);
        d->visible->setIcon(d->invisibleIcon);
    }else{
        QLineEdit::setEchoMode(QLineEdit::Normal);
        d->visible->setIcon(d->visibleIcon);
    }
}

void PasswordEdit::paintEvent(QPaintEvent *e)
{

    Q_UNUSED(e);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_FrameLineEdit, &opt, &p, this);
    QLineEdit::paintEvent(e);
}

}
