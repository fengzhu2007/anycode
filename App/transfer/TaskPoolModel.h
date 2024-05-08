#ifndef TASKPOOLMODEL_H
#define TASKPOOLMODEL_H
#include "global.h"
#include <QAbstractItemModel>
#include <QMutex>
#include <QWaitCondition>
#include <QFileIconProvider>
namespace ady {

    class Task;
    class TaskThread;

    class ANYENGINE_EXPORT TaskPoolItem
    {
    public:
        enum TaskStatus {
            Normal=0,
            Transfer,
            Failed
        };
        TaskPoolItem(Task* task, TaskPoolItem *parentItem = nullptr);
        TaskPoolItem(long long siteid,QString name, TaskPoolItem *parentItem = nullptr);

        inline Task* getTask(){return this->m_task;};
        inline TaskPoolItem* parentItem(){return this->m_parentItem;};
        int childCount();
        int childCount(TaskStatus status);
        int columnCount();
        int row() const;
        int indexOf(Task* task);
        TaskPoolItem* pop(TaskStatus status=Normal);

        QVariant data(int col);


        TaskPoolItem* child(int row);
        QList<TaskPoolItem*> children(TaskStatus status);


        void append(TaskPoolItem* item,TaskStatus status=Normal);
        void append(Task* task,TaskStatus status=Normal);
        void insertFront(TaskPoolItem* item,TaskStatus status=Normal);
        void insertFront(Task* task,TaskStatus status=Normal);
        void moveTo(TaskPoolItem* item,TaskStatus status=Normal);

        int  remove(Task* task);
        TaskPoolItem* remove(int row);
        QList<TaskPoolItem*> remove(QList<int> rows);
        QList<TaskPoolItem*> removeAll();
        QList<TaskPoolItem*> removeAll(TaskStatus status);

        void clear();

        ~TaskPoolItem();

        inline long long dataId(){return id;}
        inline long long siteId(){return siteid;}
        inline QString dataName(){return name;}
    private:
        QList<TaskPoolItem*> m_childItems;
        QList<TaskPoolItem*> m_failedItems;
        QList<TaskPoolItem*> m_transferringItems;
        Task* m_task;
        TaskPoolItem *m_parentItem;
        QString name;
        long long id;
        long long siteid;
    };


    class ANYENGINE_EXPORT TaskPoolModel : public QAbstractItemModel
    {
        Q_OBJECT
    public:
        enum Column{
            Name=0,
            Action,
            Filesize,
            Source,
            Destination,
            Status,
            Max
        };
        static TaskPoolModel* getInstance();
        static void initialize(QObject* parent=nullptr);
        void appendSite(long long siteid,QString name);
        void appendTasks(QList<Task*>lists);
        void removeSite(long long siteid);
        void clear();
        void removeAll(long long siteid=0);//remove All task
        void removeFailed(long long siteid=0);
        void retryFailed(long long siteid=0);
        void removeSelectedTasks();
        void retryFailedSelectedTasks();


        Task* take(long long siteid=0);
        TaskPoolItem* siteItem(long long siteid);
        ~TaskPoolModel();

        QVariant data(const QModelIndex &index, int role) const override;
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
        QModelIndex index(int row, int column,const QModelIndex &parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex &index) const override;
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;


        //thread
        void start();
        void start(int num);
        void stop();

        void setOnlineState(bool isOnline);

        inline bool isStarting(){return m_starting;}


    signals:
        void transferSuccess(const QString& name);

    public slots:
        void onThreadFinished();
        void removeTask(Task* task);
        void insertFront(QList<Task*>lists);
        void failTask(Task* task);
        void progressTask(Task* task);


    private:
        explicit TaskPoolModel(QObject *parent = 0);
        Task* get(long long siteid);

    private:
        QMutex* m_mutex;
        QWaitCondition* m_cond;
        TaskPoolItem* m_rootItem;
        static TaskPoolModel* instance;
        QFileIconProvider* m_iconProvider;
        QList<TaskThread*> m_threads;
        QList<TaskPoolItem*> m_waitDeleter;
        bool m_starting;
        bool m_isOnline;
    };
}
#endif // TASKPOOLMODEL_H
