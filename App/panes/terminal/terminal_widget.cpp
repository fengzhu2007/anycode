#include "terminal_widget.h"

#include <QShowEvent>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QApplication>
#include <QClipboard>
#include <ptyqt.h>
#include <array>
#include "w_toast.h"
#include "core/theme.h"
namespace ady{



class TerminalWidgetPrivate{
public:
    QString executable;
    QString workingDir;
    IPtyProcess* process = nullptr;
    bool initialized = false;

};

TerminalWidget::TerminalWidget(const QString& executable,const QString& workingDir,QWidget* parent)
    :TerminalView(parent){
    d = new TerminalWidgetPrivate;
    d->executable = executable;
    d->workingDir = workingDir;
    d->process = nullptr;


    auto colors = std::array<QColor,20>();
    colors[0] = Qt::black;
    colors[1] = 0xac4142;
    colors[2] = 0x7e8e50;
    colors[3] = 0xe5b567;
    colors[4] = 0x6c99bb;
    colors[5] = 0xa320ac;
    colors[6] = 0x7dd6cf;
    colors[7] = 0xd0d0d0;
    colors[8] = 0x505050;
    colors[9] = 0xd05e5b;
    colors[10] = 0xa7b773;
    colors[11] = 0xffd184;
    colors[12] = 0x94c8ea;
    colors[13] = 0xf257fb;
    colors[14] = 0xa1fcf7;
    colors[15] = Qt::white;



    /*for(int i=0;i<16;i++){
        colors[i] = Qt::black;
    }*/
    auto theme = Theme::getInstance();

    //custome
    colors[Foreground] = theme->textColor();
    colors[Background] = theme->backgroundColor();//background
    colors[Selection] = theme->secondaryTextColor();//selection background
    colors[FindMatch] = theme->primaryTextColor();//FindMatch background
    this->setColors(colors);

}

TerminalWidget::~TerminalWidget(){
    isDestory = true;
    if(d->process!=nullptr){
        d->process->kill();
        delete d->process;
    }
    delete d;
    d = nullptr;
}

QString& TerminalWidget::workingDir() const {
    return d->workingDir;
}

QString& TerminalWidget::executablePath() const {
    return d->executable;
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
    if(!d->initialized){
        d->initialized = true;
        this->setFocus();
    }
}

void TerminalWidget::onProcessClose(){
    if(d->process){


    }
}

void TerminalWidget::showEvent(QShowEvent* e){
    TerminalView::showEvent(e);

    QTimer::singleShot(100,[this]{
        if(isDestory){
            return ;
        }
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

    });




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

