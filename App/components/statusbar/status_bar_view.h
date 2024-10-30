#ifndef STATUSBARVIEW_H
#define STATUSBARVIEW_H
#include <QWidget>
namespace Ui {
    class StatusBarViewUi;
}
namespace ady{
    class StatusBarView : public QWidget
    {
        Q_OBJECT
    public:
        StatusBarView(QWidget* parent=nullptr);
        bool networkStatus();
        void setNetworkStatus(bool isOnline);
        void showMessage(const QString& message);
        QString currentMessage();

    private:
        Ui::StatusBarViewUi* ui;
        bool m_isOnline;

    };


}


#endif // STATUSBARVIEW_H
