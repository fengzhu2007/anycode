#ifndef SITEFRAMEWORKWIDGET_H
#define SITEFRAMEWORKWIDGET_H
#include "global.h"
#include <QWidget>
#include "storage/site_storage.h"
namespace Ui {
class SiteFrameworkWidgetUI;
}

namespace ady {
    class FormPanel;
    class ANYENGINE_EXPORT SiteFrameworkWidget : public QWidget
    {
        Q_OBJECT
    public:
        SiteFrameworkWidget(QWidget* parent,long long pid=0);
        void initView();
        void initData();
        void getFormData(SiteRecord& record);
        void setFormData(SiteRecord record);
        void setPid(long long pid);
        void clearFormData();

        inline long long dataId(){return this->id;}

        bool validateFormData(SiteRecord& record);


    public slots:
        void onTypeChanged(int index);
        void onSave();
        void onConnect();

    private:
        Ui::SiteFrameworkWidgetUI *ui;
        long long id;
        long long pid;
        QList<FormPanel*> panels;
        SiteRecord record;
    };
}
#endif // SITEFRAMEWORKWIDGET_H
