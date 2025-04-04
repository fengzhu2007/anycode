#ifndef PROJECTMANAGEMODEL_H
#define PROJECTMANAGEMODEL_H
#include "global.h"
#include "components/listview/listview_model.h"
#include "storage/project_storage.h"
#include <QFrame>


namespace ady{
class ProjectItemWidgetPrivate;
class ANYENGINE_EXPORT ProjectItemWidget : public ListViewItem{
    Q_OBJECT
public:
    ProjectItemWidget(QWidget* parent);
    ~ProjectItemWidget();
    void setTitle(const QString& title);
    void setDescription(const QString& description);
    void setIndex(int i);
public slots:
    void onEditClicked();

signals:
    void editClicked(int i);

private:
    ProjectItemWidgetPrivate *d;
};




class ProjectManageModelPrivate;
class ANYENGINE_EXPORT ProjectManageModel : public ListViewModel{
    Q_OBJECT
public:
    ProjectManageModel(ListView* parent);
    virtual int count() override;
    virtual ListViewItem* item(int i)  override;
    virtual void itemRemoved(int i) override;
    void setDataSource(QList<ProjectRecord> list);
    ProjectRecord itemAt(int i);

signals:
    void editClicked(int i);
private:
    ProjectManageModelPrivate* d;

};

}

#endif // PROJECTMANAGEVIEW_H
