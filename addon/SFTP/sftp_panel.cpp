#include "sftp_panel.h"
#include "ui_sftp_panel.h"
#include "network/network_manager.h"
#include "sftp.h"
#include "sftp_response.h"
#include <QThread>
#include <QLineEdit>
#include <QMenu>
#include <QCursor>
#include <QLabel>
#include "interface/remote_file_item_model.h"
#include "interface/NewFileDialog.h"
#include "components/message_dialog.h"
#include "components/Notification.h"
#include "transfer/Task.h"
#include "chmod_dialog.h"
#include "sftp_worker.h"
#include "sftp_setting.h"
#include "common.h"


#include <QDebug>
namespace ady{


SFTPPanel::SFTPPanel(long long id,QWidget* parent)
    :Panel(id,parent),
    ui(new Ui::SFTPPanel)
{
    ui->setupUi(this);

    /*QLabel* label = new QLabel(this);
    label->setText("teststetste");
    label->setGeometry()*/

    //this->layout()->addWidget(label);
    m_setting = nullptr;


    this->sftp = dynamic_cast<SFTP*>(this->request());
    this->worker = new SFTPWorker(id);
    this->worker->start();



    ui->treeView->header()->setSortIndicator(0,Qt::AscendingOrder);

    RemoteFileItemModel* model = new RemoteFileItemModel(id,ui->treeView);
    connect(model,SIGNAL(cellEditing(const QModelIndex&,QString)),this,SLOT(onRename(const QModelIndex&,QString)));


    ui->treeView->setModel(model);
    ui->treeView->setColumnWidth(0,180);
    ui->treeView->setColumnWidth(1,80);
    ui->treeView->setColumnWidth(2,125);
    ui->treeView->addMimeType(MM_UPLOAD);
    ui->treeView->setSupportDropFile(true);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setEditTriggers(QAbstractItemView::EditKeyPressed);

    this->dirRender = new QLineEdit(ui->toolBar);
    this->dirRender->setReadOnly(true);
    ui->toolBar->addWidget(dirRender);
    this->dirRender->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);


    connect(this->worker,SIGNAL(taskFinished(QString,NetworkResponse*)),this,SLOT(workTaskFinished(QString,NetworkResponse*)));
    connect(this->worker,&QThread::finished,this->worker,&QObject::deleteLater);


    connect(ui->treeView,SIGNAL(activated(const QModelIndex &)),this,SLOT(onItemActivated(const QModelIndex &)));
    //tree view drop upload
    connect(ui->treeView,SIGNAL(dropFinished(const QMimeData*)),this,SLOT(onDropUpload(const QMimeData*)));
    connect(ui->treeView,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(onContextMenu(const QPoint&)));

    //tree view sorting
    connect(ui->treeView->header(),SIGNAL(sortIndicatorChanged(int ,Qt::SortOrder)),model,SLOT(sortList(int,Qt::SortOrder)));

    connect(ui->actionRefresh,&QAction::triggered,this,&SFTPPanel::onActRefresh);



}

SFTPPanel::~SFTPPanel()
{
    delete ui;
    if(m_setting!=nullptr){
        delete m_setting;
    }
    if(this->worker!=nullptr){
        this->worker->stop();
        this->worker->quit();
        this->worker->wait();
        //delete this->worker;
    }
}

void SFTPPanel::init(SiteRecord record)
{
    Panel::init(record);
    m_setting = new SFTPSetting(record.data);
    m_dirSync = m_setting->dirSync();
    m_dirMapping = m_setting->dirMapping();

    this->sftp->setHost(this->site.host);
    this->sftp->setPort(this->site.port);
    this->sftp->setUsername(this->site.username);
    this->sftp->setPassword(this->site.password);
    if(!this->site.path.endsWith("/")){
        this->site.path += "/";
    }
    this->sftp->setUploadCommands(m_setting->uploadCommands());
    this->worker->append(SFTPWorker::W_LINK,0);
    //this->worker->append(SFTPWorker::W_PASV,true);

}



NetworkRequest* SFTPPanel::request()
{
    /*NetworkRequest* req = NetworkManager::getInstance()->request(this->getId());
    if(req==nullptr){
        req = NetworkManager::getInstance()->newRequest<SFTP>(this->getId());
    }
    return req;*/
    return nullptr;
}

void SFTPPanel::deleteFiles(QList<FileItem>items)
{
    DeleteTaskData* data = new DeleteTaskData;
    int size = items.size();
    data->lists.reserve(size);
    QString filename;
    for(int i=0;i<size;i++){
        FileItem item = items.at(i);
        data->lists.push_back(item);
        if(filename.isEmpty()){
            filename = item.name;
        }
    }
    long long nid = 0l;
    if(size==1){
         nid = this->notify(tr("Deleting %1").arg(filename),NotificationItem::Info);
    }else{
        nid = this->notify(tr("%1 files such as %2 are being deleted").arg(size).arg(filename),NotificationItem::Info);
    }
    data->nid = nid;
    this->worker->append(SFTPWorker::W_DELETE,(long long)data);
}





void SFTPPanel::workTaskFinished(QString name,NetworkResponse* response)
{
    if(response==nullptr){
        return ;
    }
    if(name==SFTPWorker::W_LINK){
        if(response->errorCode==0){
            this->worker->append(SFTPWorker::W_CHDIR,this->site.path);
        }else{
            //connect failed
            //response->debug();
            MessageDialog::error(ui->treeView,tr("SFTP connect failed,error :%1").arg(response->errorMsg));
        }
    }else if(name==SFTPWorker::W_CHDIR){
        //response->debug();
        if(response->errorCode==0){
            if(response->networkErrorCode==0){
                QList<FileItem> lists = response->parseList();
                QString dir = response->params["dir"].toString();
                this->currentDir = dir;
                this->dirRender->setText(dir);
                FileItem item;
                item.type = FileItem::Dir;
                item.name = "..";
                item.path = dir.mid(0,dir.lastIndexOf("/",-2) + 1);
                if(dir==this->site.path){
                    item.enabled = false;
                }
                lists.insert(0,item);
                RemoteFileItemModel* model = static_cast<RemoteFileItemModel*>(ui->treeView->model());
                model->setList(lists);
                ui->treeView->scrollToTop();
            }else{
                qDebug()<<"net error code invalid";
                response->debug();
                MessageDialog::error(ui->treeView,tr("List render failed,error :%1").arg(response->networkErrorMsg));
            }
        }else{
            response->debug();
            MessageDialog::error(ui->treeView,tr("SFTP request list folder failed,error :%1").arg(response->errorMsg));
        }

    }else if(name==SFTPWorker::W_UNLINK){

    }else if(name==SFTPWorker::W_DELETE){
        int fileCount = response->params["fileCount"].toInt();
        int folderCount = response->params["folderCount"].toInt();
        long long nid = response->params["nid"].toLongLong();
        if(fileCount==0 && folderCount==0){
            this->notify(tr("Delete file(s) or folder(s) failed"),NotificationItem::Error,2*1000,nid);
        }else{
            this->notify(tr("Delete %1 file(s) and %2 folder(s) succeeded").arg(fileCount).arg(folderCount),NotificationItem::Success,2*1000,nid);
            this->onActRefresh();
        }
    }else if(name==SFTPWorker::W_CHMOD){
        int fileCount = response->params["fileCount"].toInt();
        long long nid = response->params["nid"].toLongLong();
        if(fileCount==0){
            this->notify(tr("Chmod failed"),NotificationItem::Error,2*1000,nid);
        }else{
            this->notify(tr("Chmod succeeded on %1 files").arg(fileCount),NotificationItem::Success,2*1000,nid);
            this->onActRefresh();
        }
    }else if(name==SFTPWorker::W_MKDIR){
        this->onActRefresh();
    }else if(name==SFTPWorker::W_RENAME){
        if(response->status()){
            //refresh
            this->onActRefresh();
        }
    }
    delete  response;
}


void SFTPPanel::onItemActivated(const QModelIndex &index)
{
    int row = index.row();
    RemoteFileItemModel* model = static_cast<RemoteFileItemModel*>(ui->treeView->model());
    FileItem item = model->getItem(row);
    if(item.enabled && item.type==FileItem::Dir){
        this->worker->append(SFTPWorker::W_CHDIR,item.path);
    }
}

void SFTPPanel::onDropUpload(const QMimeData* data)
{
    if(data->hasFormat(MM_UPLOAD)){
        QByteArray b = data->data(MM_UPLOAD);
        QDataStream out(&b,QIODevice::ReadOnly);
        qint64 p;
        out >> p;
        QList<FileItem>* lists = (QList<FileItem>*)(p);

        QList<FileItem>::iterator iter = lists->begin();
        QString siteCurrentDir = this->dirRender->text();
        if(!siteCurrentDir.endsWith("/")){
            siteCurrentDir += "/";
        }
        QList<Task*> tasks ;
        while(iter!=lists->end()){
            Task* t = new Task;
            t->type = (*iter).type==FileItem::Type::File?0:1;//0=file 1=dir
            t->cmd = UPLOAD;
            t->local = (*iter).path;
            t->remote = siteCurrentDir + (*iter).name;
            t->filesize = (*iter).size;
            t->siteid = this->id;
            tasks.push_back(t);
            iter++;
        }
        delete lists;

        if(tasks.size()>0)
        {
            emit command(tasks);
        }

        //delete lists;
    }else if(data->hasUrls()){
        //drop system files
        QString siteCurrentDir = this->dirRender->text();
        if(!siteCurrentDir.endsWith("/")){
            siteCurrentDir += "/";
        }
        QList<QUrl> urls = data->urls();
        QList<Task*> tasks ;
        QList<QUrl>::iterator iter = urls.begin();
        while(iter!=urls.end()){
            QString path = (*iter).path().mid(1);
            QFileInfo fi(path);
            if(fi.exists()){
                Task* t = new Task;
                t->type = fi.isDir()==false?0:1;//0=file 1=dir
                t->cmd = UPLOAD;
                t->local = path;
                t->remote = siteCurrentDir + fi.fileName();
                if(t->type==0){
                     t->filesize = fi.size();
                }
                t->siteid = this->id;
                tasks.push_back(t);
            }
            iter++;
        }
        if(tasks.size()>0){
            emit command(tasks);
        }
    }
}

void SFTPPanel::onContextMenu(const QPoint& pos)
{
    Q_UNUSED(pos);
    QMenu* menu = new QMenu;
    menu->addAction(tr("Download"),this,SLOT(onActDownload()));
    menu->addSeparator();
    menu->addAction(tr("Rename"),this,SLOT(onActRename()));
    menu->addAction(tr("Chmod"),this,SLOT(onActChmod()));
    menu->addSeparator();
    menu->addAction(tr("New Folder"),this,SLOT(onActMkdir()));
    menu->addSeparator();
    menu->addAction(tr("Delete"),this,SLOT(onActDelete()));
    menu->exec(QCursor::pos());
}


void SFTPPanel::onActDownload()
{
    QList<FileItem> items;
    QModelIndexList indexes = ui->treeView->selectionModel()->selectedRows();
    RemoteFileItemModel* model = static_cast<RemoteFileItemModel*>(ui->treeView->model());
    int size = indexes.size();
    for(int i=0;i<size;i++){
        int row = indexes[i].row();
        items.push_back(model->getItem(row));
    }
    QList<Task*> tasks = this->matchFilePath(items,1);
    emit command(tasks);
}

void SFTPPanel::onActChmod()
{
    QModelIndexList indexes = ui->treeView->selectionModel()->selectedRows();
    int size = indexes.size();
    if(size>0){
        int mode = 0;
        RemoteFileItemModel* model = static_cast<RemoteFileItemModel*>(ui->treeView->model());
        int row = indexes[0].row();
        FileItem item = model->getItem(row);
        if(size==1){
            if(item.permission.isEmpty()==false){
                mode = ChmodDialog::toFormat(item.permission);
            }
        }
        ChmodDialog* dialog = new ChmodDialog(mode,this->topLevelWidget());
        dialog->setFileInfo(item.name,size);
        if(dialog->exec()==QDialog::Accepted){
            int mode = dialog->mode();
            int applyChildren = dialog->applyChildren();
            delete  dialog;

            ChmodTaskData* data = new ChmodTaskData;
            data->mode = mode;
            data->applyChildren = applyChildren;
            data->lists.reserve(size);
            QString filename;
            for(int i=0;i<size;i++){
                int row = indexes[i].row();
                FileItem item = model->getItem(row);
                data->lists.push_back(item);
                if(filename.isEmpty()){
                    filename = item.name;
                }
            }
            long long nid = 0l;
            if(size==1){
                nid = this->notify(tr("Chmod %1").arg(filename),NotificationItem::Info);
            }else{
                nid = this->notify(tr("%1 files such as %2 are being chmod").arg(size).arg(filename),NotificationItem::Info);
            }
            data->nid = nid;
            this->worker->append(SFTPWorker::W_CHMOD,(long long)data);
        }
    }
}

void SFTPPanel::onActDelete()
{
    if(MessageDialog::confirm(nullptr,tr("Are you confirm delete selected items?"))==QMessageBox::Yes){
        QModelIndexList indexes = ui->treeView->selectionModel()->selectedRows();
        RemoteFileItemModel* model = static_cast<RemoteFileItemModel*>(ui->treeView->model());
        int size = indexes.size();
        if(size>0){
            DeleteTaskData* data = new DeleteTaskData;
            data->lists.reserve(size);
            QString filename;
            for(int i=0;i<size;i++){
                int row = indexes[i].row();
                FileItem item = model->getItem(row);
                data->lists.push_back(item);
                if(filename.isEmpty()){
                    filename = item.name;
                }
            }
            long long nid = 0l;
            if(size==1){
                 nid = this->notify(tr("Deleting %1").arg(filename),NotificationItem::Info);
            }else{
                nid = this->notify(tr("%1 files such as %2 are being deleted").arg(size).arg(filename),NotificationItem::Info);
            }
            data->nid = nid;
            this->worker->append(SFTPWorker::W_DELETE,(long long)data);
        }
    }
}

void SFTPPanel::onActRefresh()
{
    this->worker->append(SFTPWorker::W_CHDIR,this->currentDir);
}

void SFTPPanel::onActRename(){
    QModelIndex index = ui->treeView->currentIndex();
    ui->treeView->edit(index);
}

void SFTPPanel::onActMkdir(){
    NewFileDialog* dialog = new NewFileDialog(NewFileDialog::Folder,this->topLevelWidget());
    dialog->setPath(currentDir);
    int ret = dialog->exec();
    if(ret == QDialog::Accepted){
        QString filename = dialog->fileName();
        this->worker->append(SFTPWorker::W_MKDIR,filename);
        /*QFileInfo fi(filename);
        if(fi.absoluteDir().mkdir(fi.fileName())){
            this->onActRefresh();
        }else{
            MessageDialog::error(this->topLevelWidget(),tr("Folder creation failed"));
        }*/
    }
    delete dialog;
}

void SFTPPanel::onRename(const QModelIndex& index,QString newString)
{
    //qDebug()<<"name:"<<newString;
    QRegExp reg("^[^\?\\*|\"<>:/]{1,256}$");
    //qDebug()<<"match:"<<reg.exactMatch(newString);
    if(!reg.exactMatch(newString)){
        return ;
    }
    RemoteFileItemModel* model = static_cast<RemoteFileItemModel*>(ui->treeView->model());
    //qDebug()<<"row:"<<index.row();
    FileItem item = model->getItem(index.row());
    QString src = item.path;
    if(src.endsWith("/")){
        src = src.left(src.length() - 1);
    }
    int pos = src.lastIndexOf("/");
    QString dst = src.left(pos) + "/" +newString ;
    QMap<QString,QVariant> data;
    data.insert("source",src);
    data.insert("dst",dst);
    this->worker->append(SFTPWorker::W_RENAME,data);
}


}
