#include "frames_viewer_pane.h"
#include "ui_frames_viewer_pane.h"
#include "animation_frames_player.h"
#include "frames_model.h"
#include "frames_merge_dialog.h"
#include <docking_pane_layout_item_info.h>
#include <w_toast.h>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QDir>
#include <QTimer>
#include <QMenu>
#include <QStandardPaths>
#include <QFileDialog>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QClipboard>
#include "components/message_dialog.h"
#include <docking_pane_container.h>
#include <docking_pane_container_tabbar.h>
namespace ady{

const QString FramesViewerPane::PANE_ID = "FramesViewer_%1";
const QString FramesViewerPane::PANE_GROUP = "FramesViewer";
int FramesViewerPane::SN = 0;




class IconTopLeftTextDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        painter->save();
        QIcon icon = index.data(Qt::DecorationRole).value<QPixmap>();
        QString text = index.data(Qt::DisplayRole).toString();
        QRect iconRect = option.rect;
        icon.paint(painter, iconRect);

        QFontMetrics fm(option.font);
        QRect textBound = fm.boundingRect(option.rect, Qt::TextWordWrap, text);
        QRect textRect = QRect(option.rect.topLeft(), textBound.size()).adjusted(5, 5, 5, 5);
        //QRect textRect;
        textRect.setSize({32,16});
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(0, 0, 0, 150));
        painter->drawRoundedRect(textRect, 3, 3);
        textRect = textRect.adjusted(2, 2, -2, -2);
        painter->setPen(Qt::white);
        painter->drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, text);

        if (option.state & QStyle::State_Selected) {
            QColor highlightColor = option.palette.highlight().color();
            highlightColor.setAlpha(120);
            painter->fillRect(option.rect, highlightColor);
        }
        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        return QSize(100, 100);
    }
};



class FramesViewerPanePrivate{
public:
    int id;
    AnimationFramesPlayer* player;
    QLabel* intervalLabel;
    QSpinBox* spinBox;
    FramesModel* model;
    QTimer timer;
    int current = 0;
    int interval = 200;
    QString folder;


};

FramesViewerPane::FramesViewerPane(QWidget *parent)
    : DockingPane(parent)
    , ui(new Ui::FramesViewerPane)
{
    d = new FramesViewerPanePrivate;
    d->id = FramesViewerPane::SN++;
    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    d->player = new AnimationFramesPlayer(widget);

    auto layout = static_cast<QVBoxLayout*>(widget->layout());
    layout->insertWidget(1,d->player,1);
    this->setWindowTitle(tr("Frames Viewer"));

    d->intervalLabel = new QLabel(tr("Interval:"),ui->toolBar);
    d->intervalLabel->setToolTip(tr("Animation interval (millisecond)"));
    d->spinBox = new QSpinBox(ui->toolBar);
    d->spinBox->setToolTip(tr("Animation interval (millisecond)"));
    d->spinBox->setMinimum(0);
    d->spinBox->setMaximum(1000 *  20);
    d->spinBox->setValue(d->interval);
    d->spinBox->setFixedHeight(20);
    ui->toolBar->insertWidget(ui->actionPlay,d->intervalLabel);
    ui->toolBar->insertWidget(ui->actionPlay,d->spinBox);
    connect(d->spinBox,QOverload<int>::of(&QSpinBox::valueChanged),this,&FramesViewerPane::onIntervalChanged);
    ui->actionStop->setEnabled(false);
    d->model = new FramesModel(ui->listView);
    ui->listView->setModel(d->model);
    ui->listView->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    ui->listView->setItemDelegate(new IconTopLeftTextDelegate(ui->listView));
    ui->listView->setStyleSheet("border-left:0;border-right:0;border-bottom:0");


    //connect(ui->load,&QPushButton::clicked,this,&FramesViewerPane::onLoad);
    connect(ui->listView,&QListView::doubleClicked,this,&FramesViewerPane::onDoubleClicked);

    connect(ui->listView,&QListView::customContextMenuRequested,this,&FramesViewerPane::onListContextMenu);
    connect(&d->timer,&QTimer::timeout,this,&FramesViewerPane::onFrameChange);
    connect(ui->actionPlay,&QAction::triggered,this,&FramesViewerPane::onActionTriggered);
    connect(ui->actionStop,&QAction::triggered,this,&FramesViewerPane::onActionTriggered);
    connect(ui->actionEnable,&QAction::triggered,this,&FramesViewerPane::onActionTriggered);
    connect(ui->actionDisable,&QAction::triggered,this,&FramesViewerPane::onActionTriggered);
    connect(ui->actionMerge,&QAction::triggered,this,&FramesViewerPane::onActionTriggered);
    connect(ui->actionExport,&QAction::triggered,this,&FramesViewerPane::onActionTriggered);
    connect(ui->actionOpenFolder,&QAction::triggered,this,&FramesViewerPane::onActionTriggered);
    connect(ui->actionCopy_Path,&QAction::triggered,this,&FramesViewerPane::onActionTriggered);

}

FramesViewerPane::~FramesViewerPane()
{
    delete ui;
    delete d;
}

void FramesViewerPane::initView(){

}

QString FramesViewerPane::id() {

    return PANE_ID.arg(d->id);
}
QString FramesViewerPane::group(){
    return PANE_GROUP;
}
FramesViewerPane* FramesViewerPane::open(DockingPaneManager* dockingManager,bool active){
    auto pane = new FramesViewerPane(dockingManager->widget());
    DockingPaneLayoutItemInfo* item = dockingManager->createPane(pane,DockingPaneManager::Center,active);
    return pane;
}

FramesViewerPane* FramesViewerPane::make(DockingPaneManager* dockingManager,const QJsonObject& data){
    auto pane = FramesViewerPane::open(dockingManager,true);
    return pane;
}

void FramesViewerPane::resizeEvent(QResizeEvent* e){
    DockingPane::resizeEvent(e);
}

void FramesViewerPane::loadFolder(const QString& folder){
    //auto folder = ui->folder->text();
    QDir dir(folder);
    auto container = this->container();
    if(container!=nullptr){
        int i = container->indexOf(this);
        if(i>=0){
            auto tabBar = container->tabBar();
            auto last = tabBar->lastVisibleTab();
            tabBar->setTabText(i,QString::fromUtf8("%1[%2]").arg(this->windowTitle()).arg(dir.dirName()));
            tabBar->setTabToolTip(i,folder);
            tabBar->ensureVisible(last);
        }
    }
    ui->actionCopy_Path->setEnabled(true);

    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg";
    dir.setNameFilters(filters);
    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name|QDir::DirsFirst|QDir::IgnoreCase);
    QStringList array;
    for(auto file:list){
        array.append(file.absoluteFilePath());
    }
    d->model->setDatasource(array);
    if(array.size()==0){
        wToast::showText(tr("No picture found."));
    }
}

void FramesViewerPane::onDoubleClicked(const QModelIndex& index){
    //d->label->setText(tr("%1/%2").arg(index.row()+1).arg(d->model->rowCount()));
    auto image = d->model->image(index.row());
    d->player->setText(tr("[%1/%2] Width:%3,Height:%4").arg(index.row()+1).arg(d->model->rowCount()).arg(image.size().width()).arg(image.size().height()));
    d->player->load(image);
}
void FramesViewerPane::onActionTriggered(){
    auto sender = this->sender();
    if(sender==ui->actionPlay){
        if(d->model->rowCount()<=0){
            return ;
        }
        ui->actionPlay->setEnabled(false);
        ui->actionStop->setEnabled(true);
        d->current = 0;
        d->timer.setInterval(d->interval);
        d->timer.setSingleShot(false);
        d->timer.start();
    }else if(sender==ui->actionStop){
        ui->actionPlay->setEnabled(true);
        ui->actionStop->setEnabled(false);
        d->timer.stop();
    }else if(sender==ui->actionOpenFolder){
        auto folder = d->folder;
        if(folder.isEmpty()){
            folder = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        }
        auto folderPath = QFileDialog::getExistingDirectory(this,tr("Open Image Frames Folder"),folder);
        if(!folderPath.isEmpty()){
            d->folder = folderPath;
            this->loadFolder(folderPath);
        }
    }else if(sender==ui->actionCopy_Path){
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(d->folder);
        wToast::showText(tr("Copy successfully!"));
    }else if(sender==ui->actionEnable){
        auto indexlist = ui->listView->selectionModel()->selectedIndexes();
        for(auto index:indexlist){
            d->model->setFrameStatus(index.row(),true);
        }
    }else if(sender==ui->actionDisable){
        auto indexlist = ui->listView->selectionModel()->selectedIndexes();
        for(auto index:indexlist){
            d->model->setFrameStatus(index.row(),false);
        }
    }else if(sender==ui->actionMerge){
        auto dialog = FramesMergeDialog::open(this);
        dialog->setModel(d->model);
        dialog->saveTo(d->folder);
        dialog->show();
    }else if(sender==ui->actionExport){
        auto folder = d->folder;
        if(folder.isEmpty()){
            folder = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        }
        auto folderPath = QFileDialog::getExistingDirectory(this,tr("Export to Folder"),folder);
        if(!folderPath.isEmpty()){
            auto list = d->model->results();
            for(auto frame:list){
                QFileInfo fi(frame.filename);
                auto newFilename = folderPath + "/" + fi.fileName();
                QFile::copy(frame.filename,newFilename);
            }
            if(list.size()>0){
                MessageDialog::info(this,tr("Successfully exported %1 files").arg(list.size()));
            }
        }
    }
}

void FramesViewerPane::onFrameChange(){
    auto total = d->model->rowCount();
    if(total<=0){
        return ;
    }
    for(int i=0;i<total;i++){
         d->current = d->current % total;
        auto frame = d->model->at(d->current);
        d->current += 1;
        if(frame.status){
            //d->label->setText(tr("%1/%2").arg(d->current).arg(d->model->rowCount()));
            d->player->setText(tr("[%1/%2] Width:%3,Height:%4").arg(d->current).arg(d->model->rowCount()).arg(frame.image.size().width()).arg(frame.image.size().height()));
            d->player->load(frame.image);
            return ;
        }
    }
    auto image = d->model->image(0);
    d->player->setText(tr("[%1/%2] Width:%3,Height:%4").arg(1).arg(d->model->rowCount()).arg(image.size().width()).arg(image.size().height()));
    d->player->load(image);
}



void FramesViewerPane::onListContextMenu(const QPoint &pos){
    QMenu contextMenu(this);

    auto view = static_cast<QAbstractItemView*>(this->sender());
    auto model = view->selectionModel();
    QModelIndex index = model->currentIndex();
    //int total = model->model()->rowCount();
    if(index.isValid()){
        contextMenu.addAction(ui->actionEnable);
        contextMenu.addAction(ui->actionDisable);
        contextMenu.exec(QCursor::pos());
    }
}

void FramesViewerPane::onIntervalChanged(int interval){
    d->interval = interval;
    d->timer.setInterval(d->interval);
}

}
