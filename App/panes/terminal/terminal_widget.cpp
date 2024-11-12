#include "terminal_widget.h"

#include <QShowEvent>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QApplication>
#include <QClipboard>
#include <ptyqt.h>
#include <array>
#include "w_toast.h"
namespace ady{



class TerminalWidgetPrivate{
public:
    QString executable;
    QString workingDir;
    IPtyProcess* process = nullptr;

};

TerminalWidget::TerminalWidget(const QString& executable,const QString& workingDir,QWidget* parent)
    :TerminalView(parent){
    d = new TerminalWidgetPrivate;
    d->executable = executable;
    d->workingDir = workingDir;
    d->process = nullptr;


    auto colors = std::array<QColor,20>();
    for(int i=0;i<20;i++){
        colors[i] = this->toQColor(i);
    }
    //custome
    colors[Background] = QColor{"#f5f5f5"};//background
    colors[Selection] = QColor{"#DBDBDC"};//selection background
    this->setColors(colors);

}

TerminalWidget::~TerminalWidget(){
    if(d->process!=nullptr){
        d->process->kill();
        delete d->process;
    }
    delete d;
}


qint64 TerminalWidget::writeToPty(const QByteArray &data){
    if(d->process!=nullptr){
        //qDebug()<<"writeToPty"<<data;
        return d->process->write(data);
    }else{
        return 0;
    }
}

void TerminalWidget::onReadReady(){
    auto data = d->process->readAll();
    this->writeToTerminal(data,false);
}

void TerminalWidget::onProcessClose(){
    qDebug()<<"onProcessClose:";
    if(d->process){


    }
}

void TerminalWidget::showEvent(QShowEvent* e){
    TerminalView::showEvent(e);
    if(d->process==nullptr){
        d->process = PtyQt::createPtyProcess(IPtyProcess::AutoPty);
        QSize size = this->geometry().size();

        auto list = QProcessEnvironment::systemEnvironment().toStringList();
        bool ret = d->process->startProcess(d->executable,{},d->workingDir,list,size.width(),size.height());
        if(ret){
            connect(d->process->notifier(),&QIODevice::readyRead,this,&TerminalWidget::onReadReady);
            connect(d->process->notifier(),&QIODevice::aboutToClose,this,&TerminalWidget::onProcessClose);

        }else{
            wToast::showText(tr("Start Failed,Error:%1").arg(d->process->lastError()));
        }
    }
}

bool TerminalWidget::event(QEvent *event){
    if (event->type() == QEvent::ShortcutOverride) {
        auto keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key()==Qt::Key_C && (keyEvent->modifiers() & Qt::ControlModifier)==Qt::ControlModifier){
            keyPressEvent(keyEvent);
            return true;
        }else
        if (keyEvent->key() == Qt::Key_Escape && keyEvent->modifiers() == Qt::NoModifier) {
            event->accept();
            return true;
        }
    }

    if (event->type() == QEvent::KeyPress) {
        auto k = static_cast<QKeyEvent *>(event);
        if (k->key() == Qt::Key_Escape) {
            handleEscKey(k);
            return true;
        }
        keyPressEvent(k);
        return true;
    }
    return TerminalView::event(event);
}


void TerminalWidget::resizePty(QSize newSize){
    //qDebug()<<"resizePty"<<newSize;
    if(d->process!=nullptr){
        d->process->resize(newSize.width(),newSize.height());
    }
}

void TerminalWidget::setClipboard(const QString &text){
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
    if (clipboard->supportsSelection())
        clipboard->setText(text, QClipboard::Selection);
}


void TerminalWidget::handleEscKey(QKeyEvent *event){
    bool sendToTerminal = true;
    bool send = false;
    if (sendToTerminal && event->modifiers() == Qt::NoModifier)
        send = true;
    else if (!sendToTerminal && event->modifiers() == Qt::ShiftModifier)
        send = true;

    if (send) {
        event->setModifiers(Qt::NoModifier);
        TerminalView::keyPressEvent(event);
        return;
    }

    if (selection()) {
        clearSelection();
    }
}



}

