#ifndef PROJECTEDITWIDGET_H
#define PROJECTEDITWIDGET_H
#include "global.h"
#include <QWidget>
#include "storage/project_storage.h"


namespace Ui {
class ProjectEditWidgetUI;
}

namespace ady {
    class ANYENGINE_EXPORT ProjectEditWidget : public QWidget
    {
        Q_OBJECT
    public:
        ProjectEditWidget(QWidget* parent);
        void getFormData(ProjectRecord& record);
        void setFormData(ProjectRecord record);
        void clearFormData();

    public slots:
        void onChoosePath();


    private:
        Ui::ProjectEditWidgetUI *ui;
        long long id;
    };
}


#endif // PROJECTEDITWIDGET_H
