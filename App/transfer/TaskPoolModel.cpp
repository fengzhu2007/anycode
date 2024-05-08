#include "Task.h"
#include "TaskPoolModel.h"
#include "TaskThread.h"
#include "utils.h"
#include <QMutexLocker>
#include <QFileInfo>
#include <QString>
#include <QTreeView>
#include <QTimer>
#include <QTime>
#include <QDebug>

#include "modules/log/Log.h"
namespace ady {

    TaskPoolModel* TaskPoolModel::instance = nullptr;


    TaskPoolItem::TaskPoolItem(Task* task, TaskPoolItem *parentItem)
    {
        QFileInfo fi(task->remote);
        this->name = fi.fileName();
        this->m_task = task;
        this->m_parentItem = parentItem;
        this->id = task->id;
        this->siteid = task->siteid;
    }

    TaskPoolItem::TaskPoolItem(long long siteid,QString name,TaskPoolItem *parentItem)
    {
        this->siteid = siteid;
        this->name = name;
        this->id = 0l;
        this->m_task = nullptr;
        this->m_parentItem = parentItem;
    }

    QVariant TaskPoolItem::data(int col)
    {
        if(m_task==nullptr){
            if(col==TaskPoolModel::Name){
                return this->name;
            }else{
                return QVariant();
            }

        }else{
            char cmd = m_task->cmd;
            if(col==TaskPoolModel::Name){
                return this->name;
            }else if(col==TaskPoolModel::Action){

                if(cmd==0){
                    return QVariant(QObject::tr("Upload"));
                }else if(cmd==1){
                    return QVariant(QObject::tr("Download"));
                }else if(cmd==2){
                    return QVariant(QObject::tr("Delete"));
                }else if(cmd==3){
                    return QVariant(QObject::tr("Chmod"));
                }else{
                    return QVariant();
                }
            }else if(col==TaskPoolModel::Filesize){
                if(m_task->type==0 && m_task->filesize>0){
                    return Utils::readableFilesize(m_task->filesize);
                }else{
                    return QVariant();
                }
            }else if(col==TaskPoolModel::Source){
                if(cmd==1){
                    //download
                    return m_task->remote;
                }else{
                    return m_task->local;
                }
            }else if(col==TaskPoolModel::Destination){
                if(cmd==1){
                    //download
                    return m_task->local;
                }else{
                    return m_task->remote;
                }
            }else if(col==TaskPoolModel::Status){
                if(m_task->status==Failed){
                    return m_task->errorMsg;
                }else{
                    if(m_task->filesize>0 && m_task->readysize>0){
                        return QString::asprintf("%.1f%%",m_task->readysize * 100.0 / m_task->filesize);
                    }else if(m_task->status==Transfer){
                        return QString("0.0%");
                    }else{
                        return QVariant();
                    }
                }

            }else{
                return QVariant();
            }
        }
    }



    int TaskPoolItem::childCount()
    {

        //qDebug()<<"childCount point:"<<this;
        return m_failedItems.size() +
                m_transferringItems.size() +
                m_childItems.size();
    }

    int TaskPoolItem::childCount(TaskStatus status)
    {
        if(status==Normal){
            return m_childItems.size();
        }else if(status==Transfer){
            return m_transferringItems.size();
        }else if(status==Failed){
            return m_failedItems.size();
        }else{
            return 0;
        }
    }

    int TaskPoolItem::columnCount()
    {
        return TaskPoolModel::Max;
    }

    TaskPoolItem* TaskPoolItem::child(int row)
    {
        if(row<0){
            return nullptr;
        }
        int size = this->m_failedItems.size();
        if(row<size){
            return m_failedItems.at(row);
        }
        row -= size;
        if(row<0){
            return nullptr;
        }

        size = this->m_transferringItems.size();
        if(row < size){
            return m_transferringItems.at(row);
        }
        row -= size;
        if(row<0){
            return nullptr;
        }

        size = this->m_childItems.size();
        if(row < size){
            return m_childItems.at(row);
        }
        return nullptr;
    }

    QList<TaskPoolItem*> TaskPoolItem::children(TaskStatus status)
    {
        if(status==TaskStatus::Failed){
            return m_failedItems;
        }else if(status==TaskStatus::Normal){
            return m_childItems;
        }else if(status==TaskStatus::Transfer){
            return m_transferringItems;
        }
    }

    int TaskPoolItem::row() const
    {
        int pos = -1;
        //qDebug()<<"name:"<<this->name;
        if (m_parentItem!=nullptr){
            pos = m_parentItem->m_failedItems.indexOf(const_cast<TaskPoolItem*>(this));
            if(pos==-1){
                pos = m_parentItem->m_transferringItems.indexOf(const_cast<TaskPoolItem*>(this));
            }
            if(pos==-1){
                pos = m_parentItem->m_childItems.indexOf(const_cast<TaskPoolItem*>(this));
            }
        }
        return pos;
    }

    int TaskPoolItem::indexOf(Task* task)
    {
        int i = 0;
        if(m_failedItems.isEmpty()==false){
            QList<TaskPoolItem*>::iterator iter = m_failedItems.begin();
            while(iter!=m_failedItems.end()){
                if((*iter)->getTask()==task){
                    return i;
                }
                i++;
                iter++;
            }
        }

        if(m_transferringItems.isEmpty()==false){
            QList<TaskPoolItem*>::iterator iter = m_transferringItems.begin();
            while(iter!=m_transferringItems.end()){
                if((*iter)->getTask()==task){
                    return i;
                }
                i++;
                iter++;
            }
        }

        if(m_childItems.isEmpty()==false){
            QList<TaskPoolItem*>::iterator iter = m_childItems.begin();
            while(iter!=m_childItems.end()){
                if((*iter)->getTask()==task){
                    return i;
                }
                i++;
                iter++;
            }
        }
        return -1;
    }

    TaskPoolItem* TaskPoolItem::pop(TaskStatus status)
    {
        if(status==Normal){
            //m_childItems.append(item);
            return m_childItems.isEmpty()==true?nullptr:m_childItems.takeFirst();
        }else if(status==Failed){
            return m_failedItems.isEmpty()==true?nullptr:m_childItems.takeFirst();
        }else if(status==Transfer){
            return m_transferringItems.isEmpty()==true?nullptr:m_childItems.takeFirst();
        }
    }

    void TaskPoolItem::append(TaskPoolItem* item,TaskStatus status)
    {
        Task* task = item->getTask();
        if(task!=nullptr){
            task->status = (char)status;
        }
        if(status==Normal){
            m_childItems.append(item);
        }else if(status==Failed){
            m_failedItems.append(item);
        }else if(status==Transfer){
            m_transferringItems.append(item);
        }
    }

    void TaskPoolItem::append(Task* task,TaskStatus status)
    {
        TaskPoolItem* item = new TaskPoolItem(task,this);
        this->append(item,status);
    }

    void TaskPoolItem::insertFront(TaskPoolItem* item,TaskStatus status)
    {
        if(status==Normal){
            m_childItems.insert(m_childItems.begin(),item);
        }else if(status==Failed){
            m_failedItems.insert(m_failedItems.begin(),item);
        }else if(status==Transfer){
            m_transferringItems.insert(m_transferringItems.begin(),item);
        }
    }

    void TaskPoolItem::insertFront(Task* task,TaskStatus status)
    {
        TaskPoolItem* item = new TaskPoolItem(task,this);
        this->insertFront(item,status);
    }

    void TaskPoolItem::moveTo(TaskPoolItem* item,TaskStatus status)
    {
        Task* t = item->getTask();
        if(t!=nullptr && t->status != status){
            if(status==Normal){
                //failed to normal
                if(t->status==2){
                    int position = m_failedItems.indexOf(item);
                    if(position>-1){
                        m_failedItems.removeAt(position);
                        this->append(item,Normal);
                    }
                }
            }else if(status==Transfer){


            }else if(status==Failed){
                //Transfer to Failed
                //item->row();
                if(t->status==1){
                    int position = m_transferringItems.indexOf(item);
                    if(position>-1){
                        m_transferringItems.removeAt(position);
                        this->append(item,Failed);
                    }
                }

            }
        }
    }

    int TaskPoolItem::remove(Task* task)
    {
        int i = 0;
        if(m_failedItems.isEmpty()==false){
            QList<TaskPoolItem*>::iterator iter = m_failedItems.begin();
            while(iter!=m_failedItems.end()){

                if((*iter)->getTask()==task){
                    delete *iter;
                    m_failedItems.erase(iter);
                    return i;
                }
                i++;
                iter++;
            }
        }

        if(m_transferringItems.isEmpty()==false){
            QList<TaskPoolItem*>::iterator iter = m_transferringItems.begin();
            while(iter!=m_transferringItems.end()){
                if((*iter)->getTask()==task){
                    delete *iter;
                    m_failedItems.erase(iter);
                    return i;
                }
                i++;
                iter++;
            }
        }

        if(m_childItems.isEmpty()==false){
            QList<TaskPoolItem*>::iterator iter = m_childItems.begin();
            while(iter!=m_childItems.end()){
                if((*iter)->getTask()==task){
                    delete *iter;
                    m_childItems.erase(iter);
                    return i;
                }
                i++;
                iter++;
            }
        }
        return -1;
    }

    TaskPoolItem* TaskPoolItem::remove(int row)
    {
        if(row<0){
            return nullptr;
        }
        TaskPoolItem* ptr = nullptr;
        int i = 0;
        QList<TaskPoolItem*>::iterator iter = m_failedItems.begin();
        while(iter!=m_failedItems.end()){
            if(row == i){
                ptr = (*iter);
                m_failedItems.erase(iter);
                return ptr;
            }
            i++;
            iter++;
        }

        iter = m_transferringItems.begin();
        while(iter!=m_transferringItems.end()){
            if(row == i){
                ptr = (*iter);
                m_transferringItems.erase(iter);
                return ptr;
            }
            i++;
            iter++;
        }

        iter = m_childItems.begin();
        while(iter!=m_childItems.end()){
            if(row == i){
                ptr = (*iter);
                m_childItems.erase(iter);
                return ptr;
            }
            i++;
            iter++;
        }
        return nullptr;
    }

    QList<TaskPoolItem*> TaskPoolItem::remove(QList<int> rows)
    {
        QList<TaskPoolItem*> lists;
        int i = 0;
        QList<TaskPoolItem*>::iterator iter = m_failedItems.begin();
        while(iter!=m_failedItems.end()){
            if(rows.contains(i)){
                lists.push_back(*iter);
                m_failedItems.erase(iter);
            }
            i++;
            iter++;
        }

        iter = m_transferringItems.begin();
        while(iter!=m_transferringItems.end()){
            if(rows.contains(i)){
                lists.push_back(*iter);
                m_transferringItems.erase(iter);
            }
            i++;
            iter++;
        }

        iter = m_childItems.begin();
        while(iter!=m_childItems.end()){
            if(rows.contains(i)){
                lists.push_back(*iter);
                m_childItems.erase(iter);
            }
            i++;
            iter++;
        }
        return lists;
    }

    QList<TaskPoolItem*> TaskPoolItem::removeAll()
    {
        QList<TaskPoolItem*> lists;
        if(m_failedItems.isEmpty()==false){
            QList<TaskPoolItem*>::iterator iter = m_failedItems.begin();
            while(iter!=m_failedItems.end()){
                lists.push_back(*iter);
                iter++;
            }
        }

        if(m_transferringItems.isEmpty()==false){
            QList<TaskPoolItem*>::iterator iter = m_transferringItems.begin();
            while(iter!=m_transferringItems.end()){
                //delete *iter;
                (*iter)->getTask()->abort = true;
                 lists.push_back(*iter);
                iter++;
            }
        }

        if(m_childItems.isEmpty()==false){
            QList<TaskPoolItem*>::iterator iter = m_childItems.begin();
            while(iter!=m_childItems.end()){
                 lists.push_back(*iter);
                iter++;
            }
        }

        m_failedItems.clear();
        m_transferringItems.clear();
        m_childItems.clear();
        return lists;

    }

    QList<TaskPoolItem*> TaskPoolItem::removeAll(TaskStatus status)
    {

        /*
            Normal=0,
            Transfer,
            Failed
        */
        QList<TaskPoolItem*> lists;
        if(status==Failed){
            if(m_failedItems.isEmpty()==false){
                QList<TaskPoolItem*>::iterator iter = m_failedItems.begin();
                while(iter!=m_failedItems.end()){
                    lists.push_back(*iter);
                    iter++;
                }
            }
            m_failedItems.clear();
        }else if(status==Transfer){
            if(m_transferringItems.isEmpty()==false){
                QList<TaskPoolItem*>::iterator iter = m_transferringItems.begin();
                while(iter!=m_transferringItems.end()){
                    //delete *iter;
                    (*iter)->getTask()->abort = true;
                     lists.push_back(*iter);
                    iter++;
                }
            }
            m_transferringItems.clear();
        }else if(status==Normal){

            if(m_childItems.isEmpty()==false){
                QList<TaskPoolItem*>::iterator iter = m_childItems.begin();
                while(iter!=m_childItems.end()){
                     lists.push_back(*iter);
                    iter++;
                }
            }
            m_childItems.clear();

        }


        return lists;
    }

    void TaskPoolItem::clear()
    {
        if(m_failedItems.isEmpty()==false){
            /*QList<TaskPoolItem*>::iterator iter = m_failedItems.begin();
            while(iter!=m_failedItems.end()){
                delete *iter;
                iter++;
            }*/
            qDeleteAll(m_failedItems.begin(),m_failedItems.end());
        }

        if(m_transferringItems.isEmpty()==false){
            QList<TaskPoolItem*>::iterator iter = m_transferringItems.begin();
            while(iter!=m_transferringItems.end()){
                //delete *iter;
                (*iter)->getTask()->abort = true;
                delete *iter;
                iter++;
            }
        }

        if(m_childItems.isEmpty()==false){
            /*QList<TaskPoolItem*>::iterator iter = m_childItems.begin();
            while(iter!=m_childItems.end()){
                delete *iter;
                iter++;
            }*/
            qDeleteAll(m_childItems.begin(),m_childItems.end());
        }

        m_failedItems.clear();
        m_transferringItems.clear();
        m_childItems.clear();
    }


    TaskPoolItem::~TaskPoolItem()
    {
        this->clear();
        if(m_task!=nullptr && m_task->abort== false){
            delete m_task;
            m_task = nullptr;
        }
        m_parentItem = nullptr;
    }




    /****************************TaskPoolModel********************************/

    TaskPoolModel::TaskPoolModel( QObject *parent)
        :QAbstractItemModel(parent)
    {
        this->m_mutex = new QMutex;
        this->m_cond = new QWaitCondition();
        this->m_rootItem = new TaskPoolItem(0,"");
        m_iconProvider = new QFileIconProvider;
        m_starting = false;
        m_isOnline = false;
        qRegisterMetaType<QVector<int>>("QVector<int>");
        qRegisterMetaType<QList<Task*>>("QList<Task*>");
    }

    TaskPoolModel::~TaskPoolModel()
    {
        m_starting = false;
        delete m_rootItem;
        delete m_iconProvider;
        //delete m_cond;
        //delete m_mutex;
    }


    TaskPoolModel* TaskPoolModel::getInstance(){
        return TaskPoolModel::instance ;
    }


    void TaskPoolModel::initialize(QObject* parent)
    {
        if(TaskPoolModel::instance==nullptr){
            TaskPoolModel::instance = new TaskPoolModel(parent);
        }
    }

    void TaskPoolModel::appendSite(long long siteid,QString name)
    {
        QMutexLocker locker(m_mutex);
        int position = m_rootItem->childCount();
        //QModelIndex rootIndex = this->index(0,0);
        QModelIndex rootIndex = createIndex(0,0,m_rootItem);
        beginInsertRows(rootIndex,position,position);
        TaskPoolItem* item = new TaskPoolItem(siteid,name,m_rootItem);
        this->m_rootItem->append(item);
        endInsertRows();
    }


    void TaskPoolModel::appendTasks(QList<Task*>lists)
    {
        if(lists.size()<=0){
            return ;
        }
        QMutexLocker locker(m_mutex);
        QList<Task*>::iterator iter = lists.begin();
        long long siteid = (*iter)->siteid;
        TaskPoolItem* parent = this->siteItem(siteid);
        if(parent!=nullptr){
            int row = parent->row();
            QModelIndex index = createIndex(row, 0, parent);
            int position = parent->childCount();
            beginInsertRows(index,position,position + lists.size() - 1);
            while(iter!=lists.end()){
                TaskPoolItem* child = new TaskPoolItem(*iter,parent);
                parent->append(child);
                iter++;
            }
            endInsertRows();
        }
        m_cond->wakeAll();
    }

    void TaskPoolModel::insertFront(QList<Task*>lists)
    {
        if(lists.size()<=0){
            return ;
        }

        QMutexLocker locker(m_mutex);

        QList<Task*>::iterator iter = lists.begin();
        long long siteid = (*iter)->siteid;
        TaskPoolItem* parent = this->siteItem(siteid);
        if(parent!=nullptr){
            int row = parent->row();

            QModelIndex index = createIndex(row, 0, parent);

            int position = parent->childCount(TaskPoolItem::Failed) + parent->childCount(TaskPoolItem::Transfer);
            beginInsertRows(index,position,position + lists.size() - 1);

            QList<Task*>::iterator iter = lists.end();
            do{
                iter--;
                TaskPoolItem* child = new TaskPoolItem(*iter,parent);
                parent->insertFront(child);
            }while(iter!=lists.begin());

            endInsertRows();
        }
        m_cond->wakeAll();
    }

    void TaskPoolModel::removeSite(long long siteid)
    {
        QMutexLocker locker(m_mutex);

        int size = m_rootItem->childCount();
        for(int i=0;i<size;i++){
            TaskPoolItem* child = m_rootItem->child(i);
            if(child!=nullptr && child->siteId()==siteid){
                //clear task list
                QModelIndex siteIndex = createIndex(i,0,child);
                int childCount = child->childCount();
                if(childCount>0){
                    beginRemoveRows(siteIndex,0,child->childCount() - 1);
                    QList<TaskPoolItem*> children = child->removeAll();
                    endRemoveRows();
                    qDeleteAll(children.begin(),children.end());
                    children.clear();
                }


                //clear site node
                QModelIndex index = createIndex(0,0,m_rootItem);
                beginRemoveRows(index,i,i);
                m_rootItem->remove(i);//remove self
                endRemoveRows();
                delete child;
                return ;
            }
        }
    }

    void TaskPoolModel::removeTask(Task* task)
    {
        QMutexLocker locker(m_mutex);
        long long siteid = task->siteid;
        TaskPoolItem* siteItem = this->siteItem(siteid);
        int position = siteItem->indexOf(task);
        //qDebug()<<"id1:"<<task->id;
        int size = siteItem->childCount();
        //qDebug()<<"position:"<<position;
        //qDebug()<<"local1:"<<task;
        if(position>-1){
            QModelIndex index = createIndex(siteItem->row(),0,siteItem);


            //for(int i=0;i<size;i++){
           //     TaskPoolItem* item = siteItem->child(i);
            //    qDebug()<<"position:"<<i<<":"<<item->getTask();
            //}
            beginRemoveRows(index,position,position);
            //QList<TaskPoolItem*>items = siteItem->childCount();

            TaskPoolItem* child = siteItem->remove(position);
            endRemoveRows();
            if(child!=nullptr){

                //show status message upload success


                //QString message = tr("%1 transfer successful at %2 on %3 ").arg()
                QString message;
                if(task->cmd==0){
                    //upload
                    message = tr("%1 uploaded successfully at %2").arg(child->dataName(),QTime::currentTime().toString("hh:mm"));
                }else if(task->cmd==1){
                    //download
                    message = tr("%1 downloaded successfully at %2").arg(child->dataName(),QTime::currentTime().toString("hh:mm"));
                }else{
                    //query
                    message = tr("%1 requested successfully at %2").arg(child->dataName(),QTime::currentTime().toString("hh:mm"));
                }
                Task* task = child->getTask();

                Log::transfer(siteItem->dataName(),task->remote,task->cmd==0?LogItem::UPLOAD:LogItem::DOWNLOAD);//push log
                emit transferSuccess(message);

                //qDebug()<<"local2:"<<child->getTask();
                //qDebug()<<"--------------------------";
                delete child;
                //qDebug()<<"remove point:"<<child;
                //qDebug()<<"id:"<<child->getTask()->id;
                //qDebug()<<"local:"<<child->getTask()->local;
                //qDebug()<<"remote:"<<child->getTask()->remote;
               // m_waitDeleter.push_back(child);
            }
        }
    }

    void TaskPoolModel::progressTask(Task* task)
    {
        QMutexLocker locker(m_mutex);
        long long siteid = task->siteid;
        TaskPoolItem* siteItem = this->siteItem(siteid);
        int position = siteItem->indexOf(task);
        if(position>-1){
            TaskPoolItem* child = siteItem->child(position);
            QModelIndex index = createIndex(position,TaskPoolModel::Status,child);
            emit dataChanged(index,index,QVector<int>(Qt::DisplayRole));
        }
    }

    void TaskPoolModel::failTask(Task* task)
    {
        QMutexLocker locker(m_mutex);
        long long siteid = task->siteid;
        TaskPoolItem* siteItem = this->siteItem(siteid);
        int position = siteItem->indexOf(task);
        if(position>-1){
            TaskPoolItem* child = siteItem->child(position);
            if(child!=nullptr){
                siteItem->moveTo(child,TaskPoolItem::Failed);
                int position2 = siteItem->indexOf(task);
                if(position2>-1){
                    if(position==position2){
                        //QModelIndex index = createIndex(position,TaskPoolModel::Status,siteItem);
                        QModelIndex index = createIndex(position,TaskPoolModel::Status,child);
                        emit dataChanged(index,index,QVector<int>(Qt::DisplayRole));
                    }else{
                        int a = position<position2?position:position2;
                        int b = position>position2?position:position2;

                        TaskPoolItem* childA = siteItem->child(a);
                        TaskPoolItem* childB = siteItem->child(b);
                        QModelIndex leftTop = createIndex(a,TaskPoolModel::Name,childA);
                        QModelIndex rightBottom = createIndex(b,TaskPoolModel::Status,childB);
                        emit dataChanged(leftTop,rightBottom,QVector<int>(Qt::DisplayRole));
                    }
                }
            }
        }
    }

    void TaskPoolModel::clear()
    {
        QMutexLocker locker(m_mutex);
        beginResetModel();
        //m_rootItem->clear();
        QList<TaskPoolItem*> lists = m_rootItem->removeAll();
        endResetModel();
        qDeleteAll(lists);
        lists.clear();
    }

    void TaskPoolModel::removeAll(long long siteid)
    {
        QMutexLocker locker(m_mutex);
        int size = m_rootItem->childCount();
        for(int i=0;i<size;i++){
            TaskPoolItem* site = m_rootItem->child(i);
            if(siteid==0 || siteid==site->siteId()){
                int childCount = site->childCount();
                if(childCount>0){
                    QModelIndex index = createIndex(i,0,site);
                    beginRemoveRows(index,0,childCount - 1);
                    QList<TaskPoolItem*> lists = site->removeAll();
                    endRemoveRows();
                    qDeleteAll(lists);
                }

            }
        }
    }

    void TaskPoolModel::removeFailed(long long siteid)
    {
        QMutexLocker locker(m_mutex);
        int size = m_rootItem->childCount();
        for(int i=0;i<size;i++){
            TaskPoolItem* site = m_rootItem->child(i);
            if(siteid==0 || siteid==site->siteId()){
                int childCount = site->childCount(TaskPoolItem::Failed);
                if(childCount>0){

                    QModelIndex index = createIndex(i,0,site);
                    beginRemoveRows(index,0,childCount - 1);
                    QList<TaskPoolItem*> lists = site->removeAll(TaskPoolItem::Failed);
                    endRemoveRows();
                    qDeleteAll(lists);
                }

            }
        }
    }

    void TaskPoolModel::retryFailed(long long siteid)
    {
        QMutexLocker locker(m_mutex);
        QMap<long long,QList<TaskPoolItem*>> data;
        int size = m_rootItem->childCount();
        for(int i=0;i<size;i++){
            TaskPoolItem* site = m_rootItem->child(i);
            long long sid = site->siteId();
            if(siteid==0 || siteid==sid){
                int childCount = site->childCount(TaskPoolItem::Failed);
                if(childCount>0){
                    //QModelIndex index = createIndex(site->row(),0,site);
                    QModelIndex index = this->index(site->row(),0);
                    data.insert(sid,QList<TaskPoolItem*>());
                    beginRemoveRows(index,0,childCount - 1);
                    data[sid] << site->removeAll(TaskPoolItem::Failed);
                    endRemoveRows();
                }
            }
        }
        if(data.size()>0){
            QMap<long long,QList<TaskPoolItem*>>::iterator iter = data.begin();
            while(iter!=data.end()){
                TaskPoolItem* site = this->siteItem(iter.key());
                QList<TaskPoolItem*> lists = iter.value();
                if(lists.size()>0){
                    int childCount = site->childCount();
                    //QModelIndex index = createIndex(site->row(),0,site);
                    QModelIndex index = this->index(site->row(),0);

                    beginInsertRows(index,childCount,lists.size() - 1);
                    QList<TaskPoolItem*>::reverse_iterator it = lists.rbegin();
                    while(it!=lists.rend()){
                        site->append((*it));
                        it++;
                    }
                    endInsertRows();
                }
                iter++;
            }
        }
        m_cond->wakeAll();
    }


    void TaskPoolModel::removeSelectedTasks(){
        QMutexLocker locker(m_mutex);
        QTreeView* treeView = (QTreeView*)QObject::parent();
        QModelIndexList lists = treeView->selectionModel()->selectedRows();
        QList<TaskPoolItem*>items;
        QList<Task*>* tasks = new QList<Task*>;
        for(auto index : lists){
            TaskPoolItem* item = (TaskPoolItem*)index.internalPointer();
            if(item->dataId()>0){
                //task
                int row = index.row();
                beginRemoveRows(index.parent(),row,row);
                TaskPoolItem* child = item->parentItem()->remove(row);
                Task* t = child->getTask();
                t->abort = true;
                tasks->push_back(child->getTask());
                endRemoveRows();
            }
        }
        qDeleteAll(items);
        QTimer* timer = new QTimer;
        timer->callOnTimeout([timer,tasks]{
            qDeleteAll(*tasks);
            delete tasks;
            timer->deleteLater();
        });
        timer->start(1*1000);
    }

    void TaskPoolModel::retryFailedSelectedTasks()
    {
        QMutexLocker locker(m_mutex);
        QTreeView* treeView = (QTreeView*)QObject::parent();
        QModelIndexList lists = treeView->selectionModel()->selectedRows();
        QMap<long long,QList<TaskPoolItem*>> data;
        for(auto index : lists){
            TaskPoolItem* item = (TaskPoolItem*)index.internalPointer();
            if(item->dataId()>0){
                Task* t = item->getTask();
                if(t->status==TaskPoolItem::Failed){
                    //task
                    t->abort = false;
                    int row = index.row();
                    beginRemoveRows(index.parent(),row,row);
                    TaskPoolItem* child = item->parentItem()->remove(row);
                    long long siteid = child->siteId();
                    if(!data.contains(siteid)){
                        data.insert(siteid,QList<TaskPoolItem*>());
                    }
                    data[siteid].push_back(child);
                    endRemoveRows();
                }
            }
        }
        if(data.size()>0){
            QMap<long long,QList<TaskPoolItem*>>::iterator iter = data.begin();
            while(iter!=data.end()){
                TaskPoolItem* site = this->siteItem(iter.key());
                QList<TaskPoolItem*> lists = iter.value();
                if(lists.size()>0){
                    int childCount = site->childCount();
                    QModelIndex index = createIndex(site->row(),0,site);
                    beginInsertRows(index,childCount,lists.size() - 1);
                    QList<TaskPoolItem*>::reverse_iterator it = lists.rbegin();
                    while(it!=lists.rend()){
                        site->append((*it));
                        it++;
                    }
                    endInsertRows();
                }
                iter++;
            }
        }
        m_cond->wakeAll();
    }



    Task* TaskPoolModel::take(long long siteid)
    {
        QMutexLocker locker(m_mutex);
        if(m_rootItem->childCount()==0 || m_starting==false || m_isOnline==false){
            m_cond->wait(m_mutex,5*1000);
        }
        Task* task = nullptr;
        while(m_starting && m_isOnline && (task=this->get(siteid))==nullptr){
           // qDebug()<<"thread wait";
            //qDeleteAll(m_waitDeleter);
            //m_waitDeleter.clear();
            m_cond->wait(m_mutex,5*1000);
        }
        return task;
    }

    Task* TaskPoolModel::get(long long siteid)
    {
        Task* task = nullptr;
        int size = this->m_rootItem->childCount();
        for(int i=0;i<size;i++){
            TaskPoolItem* child = this->m_rootItem->child(i);
            //qDebug()<<"child:"<<child->childCount();
            //qDebug()<<"child:"<<child->childCount(TaskPoolItem::Normal);

            if(child->childCount(TaskPoolItem::Normal) && (child->siteId()==siteid || siteid==0)){
                 TaskPoolItem* item = child->pop(TaskPoolItem::Normal);
                 if(item!=nullptr){
                     task = item->getTask();
                     child->append(item,TaskPoolItem::Transfer);
                     //qDebug()<<"local3:"<<task;
                     //child->moveTo(item,TaskPoolItem::Transfer);
                     break;
                 }
            }
        }
        if(task==nullptr && siteid>0l){
            task = this->get(0l);
        }
        return task;
    }


    TaskPoolItem* TaskPoolModel::siteItem(long long siteid)
    {
        int size = this->m_rootItem->childCount();
        TaskPoolItem* parent = nullptr;
        for(int i=0;i<size;i++){
            TaskPoolItem* child = this->m_rootItem->child(i);
            if(child->siteId()==siteid){
                parent = child;
                break;
            }
        }
        return parent;
    }


    QVariant TaskPoolModel::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant();
        if(role == Qt::DisplayRole){
            TaskPoolItem* item = static_cast<TaskPoolItem*>(index.internalPointer());
            return item->data(index.column());
        }else if(role == Qt::DecorationRole){
            if(index.column() == Name){
                TaskPoolItem* item = static_cast<TaskPoolItem*>(index.internalPointer());
                Task* task = item->getTask();
                if(task==nullptr){
                    return m_iconProvider->icon(QFileIconProvider::Network);
                }else{
                    if(task->type==0){
                        //file
                        return m_iconProvider->icon(QFileInfo(item->dataName()));
                    }else if(task->type==1){
                        //dir
                        return m_iconProvider->icon(QFileIconProvider::Folder);
                    }else{
                        return m_iconProvider->icon(QFileIconProvider::File);
                    }
                }
            }else{
                return QVariant();
            }
        }else{
            return QVariant();
        }
    }

    Qt::ItemFlags TaskPoolModel::flags(const QModelIndex &index) const
    {
        if (!index.isValid())
            return 0;

        return QAbstractItemModel::flags(index);
    }

    QVariant TaskPoolModel::headerData(int section, Qt::Orientation orientation,int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole){
            switch(section){
                case Name:
                    return tr("Name");
                break;
            case Action:
                    return tr("Action");
                break;
            case Filesize:
                    return tr("Filesize");
                break;
            case Source:
                    return tr("Source");
                break;
            case Destination:
                    return tr("Destination");
                break;
            case Status:
                    return tr("Status");
                break;

            }
        }
        return QVariant();
    }

    QModelIndex TaskPoolModel::index(int row, int column,const QModelIndex &parent) const
    {
        if (!hasIndex(row, column, parent))
            return QModelIndex();

        TaskPoolItem *parentItem;

        if (!parent.isValid())
            parentItem = m_rootItem;
        else
            parentItem = static_cast<TaskPoolItem*>(parent.internalPointer());

        TaskPoolItem *childItem = parentItem->child(row);
        if (childItem)
            return createIndex(row, column, childItem);
        else
            return QModelIndex();
    }

    QModelIndex TaskPoolModel::parent(const QModelIndex &index) const
    {
        if (!index.isValid())
            return QModelIndex();

        TaskPoolItem *childItem = static_cast<TaskPoolItem*>(index.internalPointer());
        //qDebug()<<"name:"<<childItem->data(Name);
        TaskPoolItem *parentItem = childItem->parentItem();

        /*if(parentItem==nullptr){
            qDebug()<<"name:"<<childItem->data(Name);
            qDebug()<<"siteid:"<<childItem->siteId();
            qDebug()<<"count:"<<childItem->childCount();
        }*/

        if (parentItem == m_rootItem || parentItem==nullptr)
            return QModelIndex();

        return createIndex(parentItem->row(), 0, parentItem);
    }

    int TaskPoolModel::rowCount(const QModelIndex &parent) const
    {
        TaskPoolItem *parentItem;
        if (parent.column() > 0)
            return 0;

        if (!parent.isValid()){
             parentItem = m_rootItem;
            return parentItem->childCount();
        }else{
            parentItem = static_cast<TaskPoolItem*>(parent.internalPointer());
            if(parentItem!=nullptr){
                return parentItem->childCount();
            }else{
                return 0;
            }
        }
    }

    int TaskPoolModel::columnCount(const QModelIndex &parent) const
    {
        if (parent.isValid())
            return static_cast<TaskPoolItem*>(parent.internalPointer())->columnCount();
        else
            return m_rootItem->columnCount();
    }

    void TaskPoolModel::start()
    {
        start(m_rootItem->childCount());
    }

    void TaskPoolModel::start(int num)
    {
        QMutexLocker locker(m_mutex);
        m_starting = true;
        int count = qMin(num,m_rootItem->childCount());
        for(int i=0;i<count;i++){
            TaskPoolItem* item = m_rootItem->child(i);
            long long siteid = item->siteId();
            TaskThread* thread = new TaskThread(siteid);
            connect(thread, &TaskThread::finished, this, &TaskPoolModel::onThreadFinished);
            connect(thread,SIGNAL(removeTask(Task*)),this,SLOT(removeTask(Task*)));
            connect(thread,SIGNAL(failTask(Task*)),this,SLOT(failTask(Task*)));
            connect(thread,SIGNAL(insertTasks(QList<Task*>)),this,SLOT(insertFront(QList<Task*>)));
            thread->start();
            m_threads.push_back(thread);
        }
        m_cond->wakeAll();
    }

    void TaskPoolModel::stop()
    {
        QMutexLocker locker(m_mutex);
        m_starting = false;

        int size = m_rootItem->childCount();
        for(int i=0;i<size;i++){
            TaskPoolItem* site = m_rootItem->child(i);
            QList<TaskPoolItem*> children = site->children(TaskPoolItem::Transfer);
            Q_FOREACH(TaskPoolItem* child,children){
                Task* task = child->getTask();
                if(task!=nullptr){
                    task->abort = true;
                }
            }
        }

    }

    void TaskPoolModel::setOnlineState(bool isOnline)
    {
        QMutexLocker locker(m_mutex);
        if(isOnline!=m_isOnline){
            m_isOnline = isOnline;
            if(m_isOnline){
                m_cond->wakeAll();
            }
        }
    }

    void TaskPoolModel::onThreadFinished()
    {
        TaskThread* thread = static_cast<TaskThread*>(sender());
        long long siteid = thread->siteId();
        QList<TaskThread*>::iterator iter = m_threads.begin();
        while(iter!=m_threads.end()){
            if(siteid == (*iter)->siteId()){
                m_threads.erase(iter);
                break;
            }
            iter++;
        }
        thread->deleteLater();
    }


}
