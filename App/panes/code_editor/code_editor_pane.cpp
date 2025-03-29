#include "code_editor_pane.h"
#include "ui_code_editor_pane.h"
//#include "code_editor.h"
#include "code_editor_manager.h"
#include "docking_pane_container.h"
#include "docking_pane_container_tabbar.h"
#include "panes/resource_manager/resource_manager_model.h"
#include "components/message_dialog.h"
#include "storage/recent_storage.h"
#include "code_lint.h"

#include <textdocument.h>
#include <textdocumentlayout.h>
#include <texteditorsettings.h>
#include <tabsettings.h>
#include <fontsettings.h>

#include <w_toast.h>



#include <QFileInfo>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QTextBlock>
#include <QPainter>
#include <QFileDialog>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QTextCodec>
#include <QTimer>
#include <QProcess>
#include <QSvgWidget>
#include <QDebug>


namespace ady{

const QString CodeEditorPane::PANE_ID = "CodeEditor_%1";
const QString CodeEditorPane::PANE_GROUP = "CodeEditor";
int CodeEditorPane::SN = 0;
int CodeEditorPane::NEW_COUNT = 0;

class CodeEditorPanePrivate{
public:
    int id;
    int state=CodeEditorPane::None;
    bool modification=false;
    bool scrollBarVisible=false;
    QString mineType;

};

CodeEditorPane::CodeEditorPane(QWidget *parent)
    :Editor(Editor::CodeEditor,parent),ui(new Ui::CodeEditorPane)
{
    d = new CodeEditorPanePrivate;
    d->id = CodeEditorPane::SN;
    auto widget = new QWidget(this);
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setStyleSheet(".ady--CodeEditorPane>#widget>#infoBar>QLabel{padding:0 2px;}");

    CodeEditorManager::getInstance()->append(this);
    CodeEditorPane::SN += 1;

    this->initView();


}


void CodeEditorPane::initView(){



    connect(ui->editor,&QPlainTextEdit::modificationChanged,this,&CodeEditorPane::onModificationChanged);
    connect(ui->editor,&QPlainTextEdit::cursorPositionChanged,this,&CodeEditorPane::onCursorPositionChanged);
    connect(ui->editor->textDocument(),&TextEditor::TextDocument::openFinishedSuccessfully,this,&CodeEditorPane::onFileOpend);


    ui->editor->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollBar->setRange(ui->editor->horizontalScrollBar()->minimum(),ui->editor->horizontalScrollBar()->maximum());
    QObject::connect(ui->scrollBar, &QScrollBar::valueChanged, ui->editor->horizontalScrollBar(), &QScrollBar::setValue);

    QObject::connect(ui->editor->horizontalScrollBar(), &QScrollBar::valueChanged, ui->scrollBar, &QScrollBar::setValue);
    QObject::connect(ui->editor->horizontalScrollBar(), &QScrollBar::rangeChanged, [=](int min, int max) {
        ui->scrollBar->setPageStep(ui->editor->viewport()->width());
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

    ui->editor->setMarksVisible(true);
}

CodeEditorPane::~CodeEditorPane(){
    auto manager = CodeEditorManager::getInstance();
    if(manager!=nullptr){
        manager->remove(this);
    }
    delete ui;
    delete d;
}

QString CodeEditorPane::id(){
    return PANE_ID.arg(d->id);
}

QString CodeEditorPane::group(){
    return PANE_GROUP;
}

QString CodeEditorPane::description(){
    return this->path();
}

void CodeEditorPane::activation(){
    ui->editor->setFocus();
    CodeEditorManager::getInstance()->setCurrent(this);
}

bool CodeEditorPane::save(bool rename){
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
        ui->editor->document()->setModified(false);

        if(source.isEmpty()){
            //new file save as
            ui->editor->configureGenericHighlighter();
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
    return ret;
}

void CodeEditorPane::contextMenu(const QPoint& pos){
    QMenu contextMenu(this);
    auto instance = CodeEditorManager::getInstance();
    if(instance!=nullptr){
        instance->tabContextMenu(this,&contextMenu);
    }else{
        return ;
    }
    contextMenu.exec(QCursor::pos());
}

QJsonObject CodeEditorPane::toJson(){
    QJsonObject data = {
        {"path",this->path()},
        {"mineType",d->mineType},
        {"line",ui->editor->firstVisibleBlockNumber()+1}
    };
    return {
        {"id",this->id()},
        {"group",this->group()},
        {"data",data}
    };
}

bool CodeEditorPane::closeEnable(){
    if(d->modification){
        auto name = this->path();
        if(name.isEmpty()){
            //new file
            name = this->windowTitle();
        }

        auto ret = MessageDialog::confirm(this,tr("Save"),tr("Save file\n%1?").arg(name),QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
        if(ret==QMessageBox::Cancel){
            return false;
        }
        if(ret==QMessageBox::Yes){
            return this->save(false);
        }
    }
    return true;
}

bool CodeEditorPane::doAction(int a){
    if(a==DockingPane::Save){
        return this->save(false);
    }else if(a==DockingPane::SaveAs){
        return this->save(true);
    }else if(a==DockingPane::Redo){

        ui->editor->redo();
    }else if(a==DockingPane::Undo){
        //qDebug()<<"Undo";
        ui->editor->undo();
    }else if(a==DockingPane::Cut){
        ui->editor->cut();
    }else if(a==DockingPane::Copy){
        ui->editor->copy();
    }else if(a==DockingPane::Paste){
        ui->editor->paste();
    }else if(a==DockingPane::Delete){
        QKeyEvent *event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
        QApplication::postEvent(ui->editor, event);
    }else if(a==DockingPane::SelectAll){
        ui->editor->selectAll();
    }else if(a==DockingPane::Print){
        //ui->editor->print();
    }
    return false;
}

void CodeEditorPane::rename(const QString& name){
    auto container = this->container();
    if(container!=nullptr){
        int i = container->indexOf(this);
        if(i>=0){
            QFileInfo fi(name);
            auto tabBar = container->tabBar();
            QString text = tabBar->tabText(i);
            tabBar->setTabText(i,fi.fileName());
            tabBar->setTabToolTip(i,name);
            this->editor()->rename(name);
        }
    }
}

void CodeEditorPane::autoSave(){
    const QString path = this->path();
    if(path.isEmpty()){
        return ;
    }
    auto instance = CodeEditorManager::getInstance();
    instance->removeWatchFile(path);
    if(this->writeFile(path)){

    }
    instance->appendWatchFile(path);
}

bool CodeEditorPane::readFile(const QString& path){
    auto doc = ui->editor->textDocumentPtr();
    QString error;
    auto ret = doc->open(&error,Utils::FilePath::fromString(path),Utils::FilePath::fromString(path));
    if(ret!=Core::IDocument::OpenResult::Success){
        wToast::showText(error);
        return false;
    }else{
        //checkSemantic
        this->checkSemantic();
    }
    return true;
}

bool CodeEditorPane::writeFile(const QString& path,bool autoSave){
    QString error;
    auto ret = ui->editor->textDocument()->save(&error,Utils::FilePath::fromString(path),autoSave);
    if(ret==false){
        wToast::showText(error);
    }else{
        //check Semantic
        this->checkSemantic();

    }
    return ret;
}

QString CodeEditorPane::path(){
    return ui->editor->textDocument()->filePath().path();
}

bool CodeEditorPane::isModified() const{
    return ui->editor->document()->isModified();
}

CodeEditorView* CodeEditorPane::editor(){
    return ui->editor;
}

int CodeEditorPane::fileState(){
    return d->state;
}
void CodeEditorPane::setFileState(int state){
    d->state |= state;
}

void CodeEditorPane::invokeFileState(){
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

bool CodeEditorPane::isModification(){
    return d->modification;
}

void CodeEditorPane::reload(){
    auto textDocument = ui->editor->textDocument();
    QString errorMsg;
    bool ret = textDocument->reload(&errorMsg);
    if(!errorMsg.isEmpty()){
        qDebug()<<"reload error:"<<errorMsg;
    }

    if(ret){
        //code lint checking
        this->checkSemantic();
    }
    this->updateInfoBar();
}



CodeEditorPane* CodeEditorPane::make(DockingPaneManager* dockingManager,const QJsonObject& data){
    auto pane = new CodeEditorPane();
    const QString path = data.find("path")->toString();
    if(!path.isEmpty()){
        QFileInfo fi(path);
        pane->setWindowTitle(fi.fileName());
        pane->readFile(path);
        auto instance = CodeEditorManager::getInstance();
        instance->appendWatchFile(path);
        {
            int line = data.find("line")->toInt(0);
            if(line>0){
                pane->editor()->gotoLine(line,0);
            }
        }
        //startup init,do not append to recent file
        return pane;
    }

    if(CodeEditorPane::NEW_COUNT==0){
        pane->setWindowTitle(QObject::tr("New File"));
    }else{
        pane->setWindowTitle(QObject::tr("New File (%1)").arg(CodeEditorPane::NEW_COUNT));
    }
    ++CodeEditorPane::NEW_COUNT;

    return pane;
}



void CodeEditorPane::onModificationChanged(bool changed){
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

void CodeEditorPane::onCursorPositionChanged(){
    auto cursor = ui->editor->textCursor();
    ui->position->setText(tr("Row:%1, Col:%2").arg(cursor.blockNumber() + 1).arg(cursor.positionInBlock()+1));
}

void CodeEditorPane::onFileOpend(){
    this->updateInfoBar();
}

void CodeEditorPane::updateInfoBar(){
    auto format = ui->editor->textDocument()->format();
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
    auto settings = ui->editor->textDocument()->tabSettings();

    if(settings.m_tabPolicy==TextEditor::TabSettings::TabPolicy::SpacesOnlyTabPolicy){
        ui->indent->setText(tr("Space"));
    }else{
        ui->indent->setText(tr("Tab"));
    }
}

void CodeEditorPane::showEvent(QShowEvent* e){
    DockingPane::showEvent(e);
    if(d->state>0){
        QTimer::singleShot(10,[this]{
            this->invokeFileState();
        });
    }
}

void CodeEditorPane::resizeEvent(QResizeEvent* e){

    int width = e->size().width();
    //
    int mw = 200;


    if(width < mw){
        ui->position->hide();
    }else{
        ui->position->show();
    }
    mw += ui->position->minimumWidth();

    if(width < mw){
        ui->charset->hide();
    }else{
        ui->charset->show();
    }
    mw += ui->charset->minimumWidth();

    if(width < mw){
        ui->indent->hide();
    }else{
        ui->indent->show();
    }
    mw += ui->indent->minimumWidth();
    if(width < mw){
        ui->br->hide();
    }else{
        ui->br->show();
    }
    mw += ui->br->minimumWidth();

    DockingPane::resizeEvent(e);
}


}
