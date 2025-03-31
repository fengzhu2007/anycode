#include "svg_editor_pane.h"
#include "panes/svg_editor/ui_svg_editor_pane.h"
#include "ui_svg_editor_pane.h"
#include "../code_editor/code_editor_pane.h"
#include "../code_editor/code_editor_manager.h"
#include "components/message_dialog.h"
#include "docking_pane_container.h"
#include "../image_editor/image_view.h"
#include "../code_editor/code_editor_view.h"

#include <docking_pane_container.h>
#include <docking_pane_container_tabbar.h>
#include "panes/resource_manager/resource_manager_model.h"
#include <w_toast.h>

#include <textdocument.h>
#include <textdocumentlayout.h>
#include <texteditorsettings.h>
#include <tabsettings.h>
#include <fontsettings.h>
#include "core/theme.h"

#include "svg_viewer.h"
#include <QShowEvent>
#include <QVBoxLayout>
#include <QSplitter>
#include <QTimer>
#include <QToolBar>
#include <QTextCodec>
#include <QPainter>
#include <QSvgRenderer>
namespace ady{

const QString SVGEditorPane::PANE_ID = "SVGEditor_%1";
const QString SVGEditorPane::PANE_GROUP = "SVGEditor";
int SVGEditorPane::SN = 0;

class SVGEditorPanePrivate{
public:
    int id;
    int state=CodeEditorPane::None;
    bool modification=false;
    bool scrollBarVisible=false;
    QString mineType;
    QString path;
    //ImageView* viewer;
    SVGViewer* viewer;
    CodeEditorView* editor;
    QSplitter* splitter;
    QToolBar* toolBar;

    QAction* zoomInAction;
    QAction* zoomOutAction;
    QAction* exportAction;
};


SVGEditorPane::SVGEditorPane(QWidget *parent)
    : Editor(SVGEditor,parent)
    , ui(new Ui::SVGEditorPane)
{
    d = new SVGEditorPanePrivate;
    d->id = SVGEditorPane::SN;
    auto widget = new QWidget(this);
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    auto theme = Theme::getInstance();
    auto color = theme->color().name(QColor::HexRgb);
    auto secondaryColor = theme->secondaryColor().name(QColor::HexRgb);
    this->setStyleSheet("#widget>#infoBar>QLabel{padding:0 2px;}"
                        ".QSplitterHandle{background-color:"+secondaryColor+"}");

    auto layout = static_cast<QVBoxLayout*>(widget->layout());

    d->splitter = new QSplitter(Qt::Vertical,widget);
    layout->insertWidget(0,d->splitter,1);

    d->viewer = new SVGViewer(d->splitter);
    d->editor = new CodeEditorView(d->splitter);
    d->splitter->addWidget(d->viewer);
    d->splitter->addWidget(d->editor);
    d->splitter->setSizes({1,1});
    d->splitter->setStretchFactor(0,1);
    d->splitter->setStretchFactor(1,1);
    //d->viewer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    CodeEditorManager::getInstance()->append(this);
    SVGEditorPane::SN += 1;

    d->toolBar = new QToolBar("Toolbar", this);
    d->toolBar->setOrientation(Qt::Vertical);
    d->toolBar->setIconSize(QSize(24, 24));
    d->zoomInAction = d->toolBar->addAction(QIcon(":/Resource/icons/ZoomIn_16x.svg"),tr("Zoom In"),this,&SVGEditorPane::onActionTriggered);
    d->zoomOutAction = d->toolBar->addAction(QIcon(":/Resource/icons/ZoomOut_16x.svg"),tr("Zoom Out"),this,&SVGEditorPane::onActionTriggered);
    d->exportAction = d->toolBar->addAction(QIcon(":/Resource/icons/ExportPerformanceReport_16x.svg"),tr("Export..."),this,&SVGEditorPane::onActionTriggered);
    d->toolBar->setMovable(true);
    d->toolBar->setFloatable(true);
    d->toolBar->setGeometry(20, 20, 30, 100);

    this->initView();
}

SVGEditorPane::~SVGEditorPane()
{
    auto manager = CodeEditorManager::getInstance();
    if(manager!=nullptr){
        manager->remove(this);
    }
    delete ui;
    delete d;
}

void SVGEditorPane::initView(){
    connect(d->editor,&QPlainTextEdit::modificationChanged,this,&SVGEditorPane::onModificationChanged);
    connect(d->editor,&QPlainTextEdit::cursorPositionChanged,this,&SVGEditorPane::onCursorPositionChanged);
    connect(d->editor->textDocument(),&TextEditor::TextDocument::openFinishedSuccessfully,this,&SVGEditorPane::onFileOpend);


    d->editor->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollBar->setRange(d->editor->horizontalScrollBar()->minimum(),d->editor->horizontalScrollBar()->maximum());
    QObject::connect(ui->scrollBar, &QScrollBar::valueChanged, d->editor->horizontalScrollBar(), &QScrollBar::setValue);

    QObject::connect(d->editor->horizontalScrollBar(), &QScrollBar::valueChanged, ui->scrollBar, &QScrollBar::setValue);
    QObject::connect(d->editor->horizontalScrollBar(), &QScrollBar::rangeChanged, [=](int min, int max) {
        ui->scrollBar->setPageStep(d->editor->viewport()->width());
        ui->scrollBar->setRange(min, max);
        if (min != max) {
            ui->scrollBar->setFixedHeight(18);
            d->scrollBarVisible = true;
        } else {
            ui->scrollBar->setFixedHeight(0);
            d->scrollBarVisible = false;
        }
    });

    auto instance = TextEditor::TextEditorSettings::instance();
    connect(instance,&TextEditor::TextEditorSettings::fontSettingsChanged,this,[this](TextEditor::FontSettings settings){
        ui->zoom->setZoom(settings.fontZoom());
    });
    //zoom
    int zoom = TextEditor::TextEditorSettings::instance()->fontSettings().fontZoom();
    ui->zoom->setZoom(zoom);
    connect(ui->zoom,&ZoomLabel::selected,this,[=](int z){
        TextEditor::TextEditorSettings::instance()->setZoom(z);
        ui->zoom->setZoom(z);
    });

    this->updateInfoBar();
}

QString SVGEditorPane::id(){
    return PANE_ID.arg(d->id);
}

QString SVGEditorPane::group() {
    return PANE_GROUP;
}

QString SVGEditorPane::description() {
    return this->path();
}

void SVGEditorPane::activation() {
    CodeEditorManager::getInstance()->setCurrent(this);
}

bool SVGEditorPane::save(bool rename) {
    QString path = this->path();
    const QString source = path;
    bool tabRename = false;
    if(rename){
        //save as
        path = QFileDialog::getSaveFileName(this, tr("Save File AS"), "", tr("All Files (*.*)"));
        tabRename = true;
    }else if(path.isEmpty()){
        //new file save
        path = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("All Files (*.*)"));
        tabRename = true;
    }
    if (path.isEmpty()) {
        return false;
    }
    //save

    auto instance = CodeEditorManager::getInstance();
    instance->removeWatchFile(this->path());
    QFileInfo fi(path);
    auto m = ResourceManagerModel::getInstance();
    auto list = m->takeWatchDirectory(fi.dir().absolutePath(),false);

    auto ret = this->writeFile(path);
    if(ret){
        //rename tab title
        //this->setToolTip(path);
        if(tabRename){
            auto container = this->container();
            if(container!=nullptr){
                int i = container->indexOf(this);
                if(i>=0){
                    auto tabBar = container->tabBar();
                    tabBar->setTabText(i,fi.fileName());
                    tabBar->setTabToolTip(i,path);
                }
            }
        }
        d->editor->document()->setModified(false);

        if(source.isEmpty()){
            //new file save as
            d->editor->configureGenericHighlighter();
        }

        //add to recent
        auto uri = QUrl(path);
        if(source!=path && uri.isLocalFile()){

            RecentStorage storage;
            if(source.isEmpty()){
                //remove
                storage.del(RecentStorage::File,source);
            }
            storage.add(RecentStorage::File,source);
        }
    }
    if(!list.isEmpty()){
        m->appendWatchDirectory(list.at(0));
    }
    instance->appendWatchFile(path);

    if(ret){
        //reload viewer
        d->viewer->setImagePath(d->path);
    }
    return ret;
}

void SVGEditorPane::contextMenu(const QPoint& pos) {
    QMenu contextMenu(this);
    auto instance = CodeEditorManager::getInstance();
    if(instance!=nullptr){
        instance->tabContextMenu(this,&contextMenu);
    }else{
        return ;
    }
    contextMenu.exec(QCursor::pos());
}

QJsonObject SVGEditorPane::toJson() {
    QJsonObject data = {
                        {"path",this->path()},
                        };
    return {
        {"id",this->id()},
        {"group",this->group()},
        {"data",data}
    };
}

bool SVGEditorPane::closeEnable() {
    return true;
}

bool SVGEditorPane::doAction(int a) {
    return false;
}

bool SVGEditorPane::readFile(const QString& path){
    d->path = path;
    d->viewer->setImagePath(path);
    d->viewer->setZoom(1);
    qDebug()<<"11"<<d->viewer->imageSize()<<d->viewer->size();
    auto size = d->viewer->originalSize();
    //ui->size->setText(tr("%1 * %2").arg(size.width()).arg(size.height()));
    //d->viewer->show();

    auto doc = d->editor->textDocumentPtr();
    QString error;
    auto ret = doc->open(&error,Utils::FilePath::fromString(path),Utils::FilePath::fromString(path));
    if(ret!=Core::IDocument::OpenResult::Success){
        wToast::showText(error);
        return false;
    }else{
        this->checkSemantic();
    }
    return true;
}

bool SVGEditorPane::writeFile(const QString& path,bool autoSave){
    QString error;
    auto ret = d->editor->textDocument()->save(&error,Utils::FilePath::fromString(path),autoSave);
    if(ret==false){
        wToast::showText(error);
    }else{
        //check Semantic
        this->checkSemantic();

    }
    return ret;
}

QString SVGEditorPane::path(){
    return d->path;
}

void SVGEditorPane::rename(const QString& nname){

}

void SVGEditorPane::autoSave(){

}

bool SVGEditorPane::isModification(){
    return d->modification;
}



int SVGEditorPane::fileState(){
    return d->state;
}
void SVGEditorPane::setFileState(int state){
    d->state |= state;
}

void SVGEditorPane::invokeFileState(){
    if((d->state & CodeEditorPane::Deleted)==CodeEditorPane::Deleted){
        const QString path = this->path();
        if(!QFileInfo::exists(path)){
            if(MessageDialog::confirm(this,tr("The file \"%1\" is no longer there. \nDo you want to keep it?").arg(path))==QMessageBox::No){
                //close pane
                auto container = this->container();
                if(container!=nullptr){
                    container->closePane(this);
                }
                return ;
            }
        }
        d->state &= (~CodeEditorPane::Deleted);
    }else if((d->state & CodeEditorPane::Changed)==CodeEditorPane::Changed){
        const QString path = this->path();
        if(MessageDialog::confirm(this,tr("\"%1\" \nThis file has been modified by another program.\n Reload?").arg(path))==QMessageBox::Yes){
            this->reload();
        }
        d->state &= (~CodeEditorPane::Changed);
    }
}


void SVGEditorPane::reload(){
    this->readFile(d->path);
}

CodeEditorView* SVGEditorPane::editor(){
    return d->editor;
}

SVGEditorPane* SVGEditorPane::make(DockingPaneManager* dockingManager,const QJsonObject& data){
    auto pane = new SVGEditorPane();
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


void SVGEditorPane::onCursorPositionChanged(){
    auto cursor = d->editor->textCursor();
    ui->position->setText(tr("Row:%1, Col:%2").arg(cursor.blockNumber() + 1).arg(cursor.positionInBlock()+1));
}

void SVGEditorPane::onFileOpend(){
    this->updateInfoBar();
}

void SVGEditorPane::onActionTriggered(){
    auto sender = this->sender();
    if(sender==d->zoomInAction){
        d->viewer->setZoom(d->viewer->zoom() + 0.1);
    }else if(sender==d->zoomOutAction){
        d->viewer->setZoom(d->viewer->zoom() - 0.1);
    }else if(sender==d->exportAction){
        //choose file name
        auto outputPath = QFileDialog::getSaveFileName(this, tr("Export To PNG"), "", tr("Images Files (*.png)"));
        if(outputPath.isEmpty()){
            return ;
        }
        QSize svgSize = d->viewer->imageSize();
        QImage image(svgSize, QImage::Format_ARGB32);
        image.fill(Qt::transparent);

        QPainter painter(&image);
        d->viewer->renderer()->render(&painter);
        painter.end();

        if (!image.save(outputPath, "PNG")) {
            qWarning("Failed to save PNG image");
        }
    }
}

void SVGEditorPane::updateInfoBar(){
    auto format = d->editor->textDocument()->format();
    QString name = format.codec->name();
    if(name==QLatin1String("System")){
        name = QLatin1String("ANSI");
    }
    ui->charset->setText(name);
    if(format.lineTerminationMode==Utils::TextFileFormat::LFLineTerminator){
        ui->br->setText(QLatin1String("LF"));
    }else if(format.lineTerminationMode==Utils::TextFileFormat::CRLFLineTerminator){
        ui->br->setText(QLatin1String("CRLF"));
    }else{
        ui->br->setText(tr("Unkown"));
    }
    auto settings = d->editor->textDocument()->tabSettings();

    if(settings.m_tabPolicy==TextEditor::TabSettings::TabPolicy::SpacesOnlyTabPolicy){
        ui->indent->setText(tr("Space"));
    }else{
        ui->indent->setText(tr("Tab"));
    }
}

void SVGEditorPane::showEvent(QShowEvent* e){
    Editor::showEvent(e);
    if(d->state>0){
        QTimer::singleShot(10,[this]{
            this->invokeFileState();
        });
    }
}

void SVGEditorPane::resizeEvent(QResizeEvent* e){
    Editor::resizeEvent(e);
}

void SVGEditorPane::onModificationChanged(bool changed){
    auto container = this->container();
    if(container!=nullptr){
        int i = container->indexOf(this);
        if(i>=0){
            auto tabBar = container->tabBar();
            QString text = tabBar->tabText(i);
            if(changed){
                if(!text.endsWith("*")){
                    text +="*";
                }
            }else{
                if(text.endsWith("*")){
                    text = text.left(text.length() - 1);
                }
            }
            d->modification = changed;
            tabBar->setTabText(i,text);
        }
    }
}

void SVGEditorPane::onZoom(int zoom){
    d->viewer->setZoom(zoom * 1.0 / 100);
}


}
