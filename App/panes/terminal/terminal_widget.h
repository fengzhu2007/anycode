#ifndef TERMINAL_WIDGET_H
#define TERMINAL_WIDGET_H
#include "terminal_view.h"

namespace ady{

class TerminalWidgetPrivate;
class TerminalWidget : public TerminalView
{
public:

    enum WidgetColorIndex {
        Foreground = ColorIndex::Foreground,
        Background = ColorIndex::Background,
        Selection,
        FindMatch,
    };
    explicit TerminalWidget(const QString& executable,const QString& workingDir,QWidget* parent);
    ~TerminalWidget();


    virtual qint64 writeToPty(const QByteArray &data) override;
public slots:
    void onReadReady();
    void onProcessClose();

protected:
    void showEvent(QShowEvent* e) override;
    bool event(QEvent *event) override;

    void resizePty(QSize newSize) override;
    void setClipboard(const QString &text) override;

private:
    void handleEscKey(QKeyEvent *event);

private:
    TerminalWidgetPrivate* d;


};
}

#endif // TERMINAL_WIDGET_H
