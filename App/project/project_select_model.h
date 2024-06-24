#ifndef PROJECTSELECTMODEL_H
#define PROJECTSELECTMODEL_H
#include "global.h"
#include "project_manage_model.h"

namespace ady{

class ProjectSelectItemWidgetPrivate;
class ANYENGINE_EXPORT ProjectSelectItemWidget : public ListViewItem{
    Q_OBJECT
public:
    ProjectSelectItemWidget(QWidget* parent);
    ~ProjectSelectItemWidget();
    void setTitle(const QString& title);
    void setDescription(const QString& description);
    void setIndex(int i);

private:
    ProjectSelectItemWidgetPrivate *d;
};



class ANYENGINE_EXPORT ProjectSelectModel : public ProjectManageModel
{
public:
    ProjectSelectModel(ListView* parent);
    virtual ListViewItem* item(int i)  override;

};
}

#endif // PROJECTSELECTMODEL_H
