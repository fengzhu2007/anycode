#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "network/NetworkManager.h"
#include "network/NetworkRequest.h"
#include "network/NetworkListen.h"
#include "project/ManagerWindow.h"
#include "project/OpenProjectWindow.h"
#include "storage/ProjectStorage.h"
#include "storage/SiteStorage.h"
#include "storage/AddonStorage.h"
#include "storage/DatabaseHelper.h"
#include "transfer/TaskPoolModel.h"
#include "interface/Panel.h"
#include "components/MessageDialog.h"
#include "view/StatusBarView.h"
#include "localdirpanel.h"
#include "AddonLoader.h"
#include "common.h"

#include "modules/help/AboutDialog.h"
#include "modules/help/DonationDialog.h"
#include "modules/export/DataExport.h"
#include "modules/import/DataImport.h"
#include "modules/log/LogViewer.h"
#include "modules/addon/AddonManageWindow.h"

#include <QDebug>
#include <QDesktopServices>


#include "cvs/svn/SvnRepository.h"

//#include "core/des.h"
namespace ady {
    MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
        , ui(new Ui::MainWindow)
    {
        ui->setupUi(this);

        this->initView();
        this->createStatusBar();
        this->initData();


        //cvs::SvnRepository* svn = new cvs::SvnRepository();
        //svn->init("");
        //svn->getInfo("D:/Qt/AnyDeployQ");

        /*QString password = "gz!12356";
        QString p1 = DES::encode(password);
        QString p2 = DES::decode(p1);
        qDebug()<<password;
        qDebug()<<p1;
        qDebug()<<p2;*/
    }


    void MainWindow::initView()
    {
        setWindowTitle(APP_NAME);
        ui->localTabWidget->addTab(this->localDirPanel = new ady::LocalDirPanel(ui->localTabWidget),tr("Local"));
        connect(ui->remoteTabWidget,SIGNAL(tabCloseRequested(int)),this,SLOT(onTabClosed(int)));
        ui->splitter->setSizes(QList<int>({1, 1}));

        connect(ui->actionNew,&QAction::triggered,this,&MainWindow::onNew);
        connect(ui->actionOpen,&QAction::triggered,this,&MainWindow::onOpen);
        connect(ui->actionClose,&QAction::triggered,this,&MainWindow::onClose);
        connect(ui->actionCVS,SIGNAL(triggered(bool)),this,SLOT(onSwitchCVS(bool)));

        ui->actionDelete->setData(RM_Selected);
        connect(ui->actionDelete,&QAction::triggered,this,&MainWindow::onActTaskRemove);
        //connect(ui->actionCVSRefresh,SIGNAL);
        //connect(ui->acti)


        ui->actionCVS->setEnabled(false);




        TaskPoolModel::initialize(ui->treeView);

        TaskPoolModel* model = TaskPoolModel::getInstance();

        ui->treeView->setModel(model);

        ui->treeView->setColumnWidth(TaskPoolModel::Name,180);
        ui->treeView->setColumnWidth(TaskPoolModel::Action,80);
        ui->treeView->setColumnWidth(TaskPoolModel::Filesize,80);
        ui->treeView->setColumnWidth(TaskPoolModel::Source,200);
        ui->treeView->setColumnWidth(TaskPoolModel::Destination,200);


         connect(ui->treeView,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(onContextMenu(const QPoint&)));

         connect(model,SIGNAL(transferSuccess(const QString&)),this,SLOT(onStatusMessage(const QString&)));




        ui->actionStart->setEnabled(true);
        ui->actionStop->setEnabled(false);
        connect(ui->actionStart,&QAction::triggered,this,&MainWindow::onStart);
        connect(ui->actionStop,&QAction::triggered,this,&MainWindow::onStop);
        connect(ui->actionQuit,&QAction::triggered,this,&MainWindow::close);
        connect(ui->actionWebsite,&QAction::triggered,this,&MainWindow::onWebsite);
        connect(ui->actionDonation,&QAction::triggered,this,&MainWindow::onDonation);
        connect(ui->actionAbout,&QAction::triggered,this,&MainWindow::onAbout);
        connect(ui->actionImport,&QAction::triggered,this,&MainWindow::onImport);
        connect(ui->actionExport,&QAction::triggered,this,&MainWindow::onExport);
        connect(ui->actionAddon,&QAction::triggered,this,&MainWindow::onAddonManage);
        connect(ui->actionLog,&QAction::triggered,this,&MainWindow::onViewLog);

        m_logViewer = nullptr;
    }

    void MainWindow::initData()
    {
        m_projectId = 0l;

        //QNetworkConfigurationManager mgr;
        //QNetworkConfigurationManager* networkConfig = new QNetworkConfigurationManager();
        //TaskPoolModel::getInstance()->setOnlineState(networkConfig->isOnline());
        //connect(networkConfig,SIGNAL(onlineStateChanged(bool)),this,SLOT(onNetworkChanged(bool)));
        //connect(networkConfig,&QNetworkConfigurationManager::updateCompleted,this,&MainWindow::onNetworkUpdate);
        //NetworkListen* listen = new NetworkListen();
        //NetworkManager::getInstance()->newRequest<NetworkListen>();

        NetworkListen* listen = new NetworkListen(this);
        connect(listen,SIGNAL(onlineStateChanged(bool)),this,SLOT(onNetworkStateChanged(bool)));

    }

    void MainWindow::createStatusBar()
    {
        statusBar = new StatusBarView(ui->statusbar);
        ui->statusbar->addWidget(statusBar,1);
        //ui->statusbar->showMessage("1111111111");
        //statusBar->showMessage("connect(networkConfig,&QNetworkConfigurationManager::updateCompleted,this,&MainWindow::onNetworkUpdate);");
    }


    Panel* MainWindow::panel(long long id)
    {
        if(m_panels.contains(id)){
            return m_panels[id];
        }else{
            return nullptr;
        }
    }

    QMap<long long,Panel*> MainWindow::panels()
    {
        return m_panels;
    }

    bool MainWindow::cvsMode()
    {
        return ui->actionCVS->isChecked();
    }



    void MainWindow::onNew()
    {
        ManagerWindow* win = new ManagerWindow(this);
        win->setModal(true);
        win->show();
    }

    void MainWindow::onOpen()
    {
        OpenProjectWindow* dialog = new OpenProjectWindow(this);
        dialog->setModal(true);
        dialog->show();
        connect(dialog,SIGNAL(selectionChanged(long long)),this,SLOT(onProjectOpened(long long)));
    }

    void MainWindow::onClose()
    {
        //qDebug()<<"onclose";
        this->onProjectClose();
    }

    void MainWindow::onProjectOpened(long long id)
    {
        this->onProjectClose();
        AddonStorage addonStorage;
        QMap<QString,QString> data = addonStorage.dictionary();
        ProjectStorage projectStorage;
        ProjectRecord r = projectStorage.one(id);
        this->localDirPanel->setProject(r);
        if(r.id>0l){
            if(!this->localDirPanel->openProjectDir(r.path)){
                MessageDialog::error(this,tr("Invalid Project Path"));
                return ;
            }
            r.updatetime = QDateTime::currentDateTime().toSecsSinceEpoch();
            projectStorage.update(r);//update project updatetime


            m_projectId = id;
            ui->actionCVS->setEnabled(true);
            this->setWindowTitle(QString("%1 - %2").arg(r.name).arg(APP_NAME));
            GroupStorage groupStorage;
            QList<GroupRecord> groups = groupStorage.list(id);
            this->localDirPanel->setGroups(groups);
            AddonLoader* loader = AddonLoader::getInstance();
            TaskPoolModel* model = TaskPoolModel::getInstance();
            SiteStorage siteStorage;
            QList<SiteRecord> sites = siteStorage.list(id,1);
            this->localDirPanel->setSites(sites);
            QList<SiteRecord>::iterator iter = sites.begin();
            while(iter!=sites.end()){
                QString name = (*iter).type;// type name
                if(data.contains(name)){
                    if(loader->load(data[name])){
                        Panel* panel = loader->getPanel((*iter).id,ui->remoteTabWidget,name);
                        if(panel!=nullptr){
                            panel->setPorjectDir(r.path);
                            //connect(panel,SIGNAL(command(CMD,long long ,QList<FileItem> ,bool)),localDirPanel,SLOT(onCommand(CMD,long long ,QList<FileItem> ,bool)));
                            connect(panel,SIGNAL(command(QList<Task*>)),localDirPanel,SLOT(onCommand(QList<Task*>)));
                            m_panels[(*iter).id] = panel;
                            panel->setGroupid((*iter).groupid);
                            panel->setType((*iter).type);
                            panel->setWindowTitle((*iter).name);
                            panel->init(*iter);
                            //panel->request();//new curl client
                            int index = ui->remoteTabWidget->addTab(panel,panel->windowTitle());
                            ui->remoteTabWidget->setTabToolTip(index,(*iter).host);
                            //add to tree view
                            model->appendSite((*iter).id,(*iter).name);
                        }
                    }
                }
                iter++;
            }
        }else{
            QList<SiteRecord> sites;
            this->localDirPanel->setSites(sites);
        }
    }

    void MainWindow::onProjectClose()
    {
        DatabaseHelper::remove(m_projectId);
        onSwitchCVS(false);
        ui->actionCVS->setChecked(false);
        ui->actionCVS->setEnabled(false);
        this->setWindowTitle(APP_NAME);
        m_panels.clear();
        //remove tabs
        int count = ui->remoteTabWidget->count();
        for(int i=0;i<count;i++){
            QWidget* w = ui->remoteTabWidget->widget(0);
            ui->remoteTabWidget->removeTab(0);
            delete w;
        }
        //clear tree view
        TaskPoolModel* model = TaskPoolModel::getInstance();
        if(model!=nullptr){
            model->clear();
        }
        //clear cvs data
        localDirPanel->onActCVSClear();
        m_projectId = 0l;


        ProjectRecord r;
        this->localDirPanel->setProject(r);
        this->localDirPanel->removeFavorite();
    }

    void MainWindow::onTabClosed(int index)
    {
        //qDebug()<<"onTabClosed:"<<index;
        QMap<long long,Panel*>::iterator iter = m_panels.begin();
        int i = 0;
        long long siteid = 0;
        while(iter!=m_panels.end()){
            if(i==index){
                siteid = iter.key();
                m_panels.erase(iter);
                break;
            }
            i++;
            iter++;
        }
        //remove tab
        //delete widget
        QWidget* w = ui->remoteTabWidget->widget(index);
        ui->remoteTabWidget->removeTab(index);
        delete w;

        TaskPoolModel* model = TaskPoolModel::getInstance();
        if(model!=nullptr ){
            model->removeSite(siteid);
        }
        //remove network
        NetworkManager::getInstance()->remove(siteid);
    }

    void MainWindow::onSwitchCVS(bool checked)
    {
        localDirPanel->onSwitchCVS(checked);
    }

    void MainWindow::onTaskExpandAll()
    {
        ui->treeView->expandAll();
    }

    void MainWindow::onStart()
    {
        ui->actionStart->setEnabled(false);
        ui->actionStop->setEnabled(true);
        TaskPoolModel::getInstance()->start();
    }

    void MainWindow::onStop()
    {
        ui->actionStart->setEnabled(true);
        ui->actionStop->setEnabled(false);
        TaskPoolModel::getInstance()->stop();
    }

    void MainWindow::onContextMenu(const QPoint& pos)
    {
        Q_UNUSED(pos);
        QMenu* menu = new QMenu;
        if(TaskPoolModel::getInstance()->isStarting()){
            menu->addAction(tr("Pause"),this,SLOT(onStop()));
        }else{
            menu->addAction(tr("Start"),this,SLOT(onStart()));
        }
        menu->addSeparator();
        QMenu* subMenu = menu->addMenu(tr("Failed"));
        subMenu->addAction(tr("Retry"),this,SLOT(onActTaskRetry()))->setData(RT_Selected);
        subMenu->addSeparator();
        subMenu->addAction(tr("Retry All"),this,SLOT(onActTaskRetry()))->setData(RT_All_Failed);
        subMenu->addAction(tr("Delete All Failed"),this,SLOT(onActTaskRemove()))->setData(RM_All_Failed);
        menu->addSeparator();
        menu->addAction(tr("Delete"),this,SLOT(onActTaskRemove()))->setData(RM_Selected);
        menu->addAction(tr("Delete All"),this,SLOT(onActTaskRemove()))->setData(RM_All);
        menu->exec(QCursor::pos());
    }

    void MainWindow::onActTaskRemove()
    {
        QAction* action = (QAction*)sender();
        int data = action->data().toInt();
        if(data==RM_Selected){
            if(MessageDialog::confirm(this,tr("Are you want delete selected tasks?"))==QMessageBox::Yes){
                TaskPoolModel::getInstance()->removeSelectedTasks();
            }
        }else if(data==RM_All){
            //remove all task items
            if(MessageDialog::confirm(this,tr("Are you want delete all tasks?"))==QMessageBox::Yes){
                TaskPoolModel::getInstance()->removeAll();
            }
        }else if(data==RM_All_Failed){
            if(MessageDialog::confirm(this,tr("Are you want delete all failed tasks?"))==QMessageBox::Yes){
                TaskPoolModel::getInstance()->removeFailed();
            }
        }
    }

    void MainWindow::onActTaskRetry()
    {
        QAction* action = (QAction*)sender();
        int data = action->data().toInt();
        if(data==RT_Selected){
            //retry selected failed task items
            TaskPoolModel::getInstance()->retryFailedSelectedTasks();
        }else if(data==RT_All_Failed){
            //retry all failed task items
             TaskPoolModel::getInstance()->retryFailed();
        }
    }

    void MainWindow::onNetworkStateChanged(bool isOnline)
    {
        TaskPoolModel::getInstance()->setOnlineState(isOnline);
        statusBar->setNetworkStatus(isOnline);
        if(isOnline==false){
            statusBar->showMessage(tr("Network exception"));
        }else{
            statusBar->showMessage("");
        }
    }

    void MainWindow::onStatusMessage(const QString & name){
        this->statusBar->showMessage(name);
    }

     void MainWindow::onDonation()
     {
         DonationDialog * dialog = new DonationDialog(this);
         dialog->show();
     }

     void MainWindow::onAbout()
     {
        AboutDialog * dialog = new AboutDialog(this);
        dialog->show();
     }

     void MainWindow::onWebsite()
     {
        QDesktopServices::openUrl(QUrl(APP_URL));
     }

     void MainWindow::onExport()
     {
        DataExport exp;
        int ret = exp.doExport(this);
        if(ret==0){
            MessageDialog::info(this,tr("Export Successful"));
        }else if(ret>0){
            MessageDialog::info(this,tr("Export Failed"));
        }
     }


     void MainWindow::onImport()
     {
        DataImport imp;
        int ret = imp.doImport(this);
        if(ret==0){
            MessageDialog::info(this,tr("Import Successful"));
        }else if(ret>0){
            MessageDialog::info(this,tr("Import Failed"));
        }
     }

     void MainWindow::onAddonManage(){
         AddonManageWindow::getInstance(this)->show();
     }

     void MainWindow::onViewLog()
     {
        if(m_logViewer==nullptr){
            m_logViewer = new LogViewer(this);
        }
        m_logViewer->show();
        m_logViewer->scrollBottom();
     }

    MainWindow::~MainWindow()
    {
        delete ui;
        NetworkManager::getInstance()->clear();
    }

}


