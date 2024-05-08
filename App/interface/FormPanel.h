#ifndef FORMPANEL_H
#define FORMPANEL_H
#include "global.h"
#include <QWidget>
#include "storage/SiteStorage.h"
namespace ady {
    class ANYENGINE_EXPORT FormPanel : public QWidget
    {
        Q_OBJECT
    public:
        FormPanel(QWidget* parent);

        virtual void initFormData(SiteRecord record) = 0;
        virtual bool validateFormData(SiteRecord& record) = 0;
        virtual bool isDataChanged();



    };
}
#endif // FORMPANEL_H
