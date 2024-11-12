#include "code_editor_pane.h"
#include "ui_code_editor_pane.h"
//#include "code_editor.h"
#include "code_editor_manager.h"
#include "docking_pane_container.h"
#include "docking_pane_container_tabbar.h"
#include "panes/resource_manager/resource_manager_model.h"
#include "components/message_dialog.h"

#include <textdocument.h>
#include <texteditorsettings.h>
#include <tabsettings.h>
#include <fontsettings.h>



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
#include <QDebug>


#define BUFFER_SIZE 65536
char buffer[BUFFER_SIZE];

namespace ady{

const QString CodeEditorPane::PANE_ID = "CodeEditor_%1";
const QString CodeEditorPane::PANE_GROUP = "CodeEditor";
int CodeEditorPane::SN = 0;

class CodeEditorPanePrivate{
public:
    //CodeEditorView* editor;
    int id;
    int state=CodeEditorPane::None;
    bool modification=false;
    bool scrollBarVisible=false;
    QString mineType;


};

CodeEditorPane::CodeEditorPane(QWidget *parent)
    :DockingPane(parent),ui(new Ui::CodeEditorPane)
{
    //Subscriber::reg();
    d = new CodeEditorPanePrivate;
    d->id = CodeEditorPane::SN;
    //d->editor = new CodeEditorView(this);
    auto widget = new QWidget(this);
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setStyleSheet(".ady--CodeEditorPane>#widget{background-color:#f5f5f5}"
                        ".ady--CodeEditorPane>#widget>#infoBar>QLabel{padding:0 2px;}");


    CodeEditorManager::getInstance()->append(this);
    CodeEditorPane::SN += 1;
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

    //zoom
    this->initView();
}


void CodeEditorPane::initView(){
    int zoom = TextEditor::TextEditorSettings::instance()->fontSettings().fontZoom();

    ui->zoom->setZoom(zoom);
}

CodeEditorPane::~CodeEditorPane(){
    //Subscriber::unReg();
    //qDebug()<<"CodeEditorPane::~CodeEditorPane";
    //qDebug()<<"CodeEditorPane::~CodeEditorPane"<<this;


    auto manager = CodeEditorManager::getInstance();
    if(manager!=nullptr){
        manager->remove(this);
    }
    //disconnect(d->editor,&QPlainTextEdit::modificationChanged,this,&CodeEditorPane::onModificationChanged);
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
    //d->editor->init();
    ui->editor->setFocus();
    //qDebug()<<"activation"<<this->id()<<this->windowTitle();
    CodeEditorManager::getInstance()->setCurrent(this);

}

void CodeEditorPane::save(bool rename){
    QString path = this->path();
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
        return ;
    }
    //save
    auto instance = CodeEditorManager::getInstance();
    instance->removeWatchFile(this->path());
    QFileInfo fi(path);
    auto m = ResourceManagerModel::getInstance();
    auto list = m->takeWatchDirectory(fi.dir().absolutePath(),false);

    if(this->writeFile(path)){
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
    }
    if(!list.isEmpty()){
        m->appendWatchDirectory(list.at(0));
    }
    instance->appendWatchFile(path);

}

void CodeEditorPane::contextMenu(const QPoint& pos){
    QMenu contextMenu;
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
        {"line",ui->editor->textCursor().block().blockNumber()+1}
    };
    return {
        {"id",this->id()},
        {"group",this->group()},
        {"data",data}
    };
}

/*bool CodeEditorPane::onReceive(Event* e){

    return false;
}*/

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

bool CodeEditorPane::readFile(const QString& path){
    auto doc = ui->editor->textDocumentPtr();
    /*if(!doc){
        doc = QSharedPointer<TextEditor::TextDocument>(new TextEditor::TextDocument(Core::Constants::K_DEFAULT_TEXT_EDITOR_ID));
        d->editor->setTextDocument(doc);
        static TextEditor::DocumentContentCompletionProvider basicSnippetProvider;
        doc->setCompletionAssistProvider(&basicSnippetProvider);
        //auto indenter = new TextEditor::TextIndenter(doc->document());
        auto indenter = new Android::Internal::JavaIndenter(doc->document());
        doc->setIndenter(indenter);
    }*/

    /*const QString charset = detectFileEncoding(path);
    if(charset.isEmpty()){
        //doc->setCodec(QTextCodec::codecForName(charset.toStdString().c_str()));
    }else{
        //doc->setCodec(QTextCodec::codecForName("UTF-8"));
    }*/

    QString error;
    auto ret = doc->open(&error,Utils::FilePath::fromString(path),Utils::FilePath::fromString(path));
    qDebug()<<path<<doc->codec()->name();
    return true;
}

bool CodeEditorPane::writeFile(const QString& path){
    QString error;
    auto ret = ui->editor->textDocument()->save(&error,Utils::FilePath::fromString(path),false);
    return true;
    //return d->editor->writeFile(path);
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
    textDocument->reload(&errorMsg);
    if(!errorMsg.isEmpty()){
        qDebug()<<"reload error:"<<errorMsg;
    }
    this->updateInfoBar();
}

CodeEditorPane* CodeEditorPane::make(DockingPaneManager* dockingManager,const QJsonObject& data){
    auto pane = new CodeEditorPane();
    auto value = data.find("path");
    if(value!=data.end()){
        const QString path = value->toString();
        if(!path.isEmpty()){
            QFileInfo fi(path);
            pane->setWindowTitle(fi.fileName());
            pane->readFile(path);
            auto instance = CodeEditorManager::getInstance();
            instance->appendWatchFile(path);

            {
                auto value = data.find("line");
                if(value!=data.end()){
                    int line = value->toInt(1);
                    pane->editor()->gotoLine(line,0);
                }
            }

            return pane;
        }
    }
    pane->setWindowTitle(QObject::tr("New File"));
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
    //ui->row->setText(tr("Line:%1").arg(cursor.blockNumber() + 1));
    //ui->column->setText(tr("Column:%1").arg(cursor.positionInBlock()+1));
    ui->position->setText(tr("Row:%1, Col:%2").arg(cursor.blockNumber() + 1).arg(cursor.positionInBlock()+1));
}

void CodeEditorPane::onFileOpend(){
    this->updateInfoBar();
}

void CodeEditorPane::updateInfoBar(){
    auto format = ui->editor->textDocument()->format();
    ui->charset->setText(format.codec->name());
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
