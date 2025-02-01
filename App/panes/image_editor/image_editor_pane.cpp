#include "image_editor_pane.h"
#include "ui_image_editor_pane.h"
#include "../code_editor/code_editor_pane.h"
#include "../code_editor/code_editor_manager.h"
#include "docking_pane_container.h"
#include "image_view.h"
namespace ady{


const QString ImageEditorPane::PANE_ID = "ImageEditor_%1";
const QString ImageEditorPane::PANE_GROUP = "ImageEditor";
int ImageEditorPane::SN = 0;

class ImageEditorPanePrivate{
public:
    int id;
    int state=CodeEditorPane::None;
    bool modification=false;
    bool scrollBarVisible=false;
    QString mineType;
    QString path;
    ImageView* viewer;
};


ImageEditorPane::ImageEditorPane(QWidget *parent):Editor(ImageEditor,parent),ui(new Ui::ImageEditorPane) {
    d = new ImageEditorPanePrivate;
    d->id = ImageEditorPane::SN;
    auto widget = new QWidget(this);
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setStyleSheet("#widget>#infoBar>QLabel{padding:0 2px;}");
    //background:repeating-linear-gradient(45deg, #ffffff, #ffffff 10px, #808080 10px, #808080 20px);
    //widget->setStyleSheet("#widget{background:repeating-linear-gradient(45deg, #808080 0%, #808080 10%, #FFFFFF 10%, #FFFFFF 20%),        repeating-linear-gradient(-45deg, #808080 0%, #808080 10%, #FFFFFF 10%, #FFFFFF 20%);}");
    CodeEditorManager::getInstance()->append(this);
    ImageEditorPane::SN += 1;

    auto layout = static_cast<QVBoxLayout*>(widget->layout());
    d->viewer = new ImageView(widget);

    layout->insertWidget(0,d->viewer,1);


    this->initView();
}

ImageEditorPane::~ImageEditorPane(){
    delete d;
}

void ImageEditorPane::initView(){
    ui->zoom->setZoom(100);
    d->viewer->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollBar->setRange(d->viewer->horizontalScrollBar()->minimum(),d->viewer->horizontalScrollBar()->maximum());
    QObject::connect(ui->scrollBar, &QScrollBar::valueChanged, d->viewer->horizontalScrollBar(), &QScrollBar::setValue);

    QObject::connect(d->viewer->horizontalScrollBar(), &QScrollBar::valueChanged, ui->scrollBar, &QScrollBar::setValue);
    QObject::connect(d->viewer->horizontalScrollBar(), &QScrollBar::rangeChanged, [=](int min, int max) {
        ui->scrollBar->setPageStep( d->viewer->viewport()->width());
        ui->scrollBar->setRange(min, max);
        if (min != max) {
            ui->scrollBar->setFixedHeight(18);
            d->scrollBarVisible = true;
        } else {
            ui->scrollBar->setFixedHeight(0);
            d->scrollBarVisible = false;
        }
    });
}

QString ImageEditorPane::id(){
    return PANE_ID.arg(d->id);
}

QString ImageEditorPane::group() {
    return PANE_GROUP;
}

QString ImageEditorPane::description() {
    return this->path();
}

void ImageEditorPane::activation() {
    //ui->editor->setFocus();
    CodeEditorManager::getInstance()->setCurrent(this);
}

void ImageEditorPane::save(bool rename) {

}

void ImageEditorPane::contextMenu(const QPoint& pos) {
    QMenu contextMenu(this);
    auto instance = CodeEditorManager::getInstance();
    if(instance!=nullptr){
        instance->tabContextMenu(this,&contextMenu);
    }else{
        return ;
    }
    contextMenu.exec(QCursor::pos());
}

QJsonObject ImageEditorPane::toJson() {
    QJsonObject data = {
        {"path",this->path()},
    };
    return {
        {"id",this->id()},
        {"group",this->group()},
        {"data",data}
    };
}

bool ImageEditorPane::closeEnable() {
    return true;
}

void ImageEditorPane::doAction(int a) {

}

bool ImageEditorPane::readFile(const QString& path){
    d->path = path;
    //ui->imageView->setPixmap(QPixmap(path));
    d->viewer->setImagePath(path);

    auto size = d->viewer->imageSize();
    ui->size->setText(tr("%1 * %2").arg(size.width()).arg(size.height()));
    return true;
}

QString ImageEditorPane::path(){
    return d->path;
}

void ImageEditorPane::rename(const QString& nname){

}

void ImageEditorPane::autoSave(){

}

bool ImageEditorPane::isModification(){
    return false;
}

int ImageEditorPane::fileState(){
    return d->state;
}
void ImageEditorPane::setFileState(int state){

}
void ImageEditorPane::invokeFileState(){

}
void ImageEditorPane::reload(){

}
CodeEditorView* ImageEditorPane::editor(){
    return nullptr;
}

ImageEditorPane* ImageEditorPane::make(DockingPaneManager* dockingManager,const QJsonObject& data){
    auto pane = new ImageEditorPane();
    const QString path = data.find("path")->toString();
    if(!path.isEmpty()){
        QFileInfo fi(path);
        pane->setWindowTitle(fi.fileName());
        pane->readFile(path);
        auto instance = CodeEditorManager::getInstance();
        instance->appendWatchFile(path);
        //startup init,do not append to recent file
        return pane;
    }
    return pane;
}

}
