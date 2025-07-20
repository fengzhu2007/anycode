#ifndef WORKFLOW_MODEL_H
#define WORKFLOW_MODEL_H


#include "global.h"
#include "components/listview/listview_model.h"
#include "image_process_thread.h"


namespace ady{

class WorkflowData{
public:
    ImageProcessThread::ProcessName name;
    QString title;
    void* data;

public:
    WorkflowData(ImageProcessThread::ProcessName name,const QString& title,void* data){
        this->name = name;
        this->title = title;
        this->data = data;
    }

    WorkflowData(const WorkflowData&);
    ~WorkflowData();
    WorkflowData& operator=(const WorkflowData&);
};

class WorkflowModelPrivate;
class ANYENGINE_EXPORT WorkflowModel : public ListViewModel{
    Q_OBJECT
public:
    WorkflowModel(ListView* parent);
    virtual int count() override;
    virtual ListViewItem* item(int i)  override;
    virtual void itemRemoved(int i) override;
    void setDataSource(QList<WorkflowData> list);
    void appendItem(const WorkflowData& item);
    WorkflowData itemAt(int i);
    QList<WorkflowData>& dataSource();

signals:
    void itemClicked(int i);
public slots:
    void onRemoved(int i);
private:
    WorkflowModelPrivate* d;

};

}

#endif // WORKFLOW_MODEL_H
