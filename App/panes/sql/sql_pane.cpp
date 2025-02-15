#include "sql_pane.h"
#include "ui_sql_pane.h"
#include "../code_editor/code_editor_manager.h"

namespace ady{

const QString SQLPane::PANE_ID = "SQL_%1";
const QString SQLPane::PANE_GROUP = "SQL";
int SQLPane::SN = 0;

class SQLPanePrivate{
public:
    int id;
    QString driver;
    QString path;
};


SQLPane::SQLPane(QWidget *parent)
    : Editor(Editor::SQLManager,parent)
    , ui(new Ui::SQLPane)
{
    ui->setupUi(this);
    d = new SQLPanePrivate();
    d->id = SQLPane::SN;
    CodeEditorManager::getInstance()->append(this);
    SQLPane::SN += 1;
}

SQLPane::~SQLPane()
{
    delete d;
    delete ui;
}

void SQLPane::initView(){

}

QString SQLPane::id() {
    return PANE_ID.arg(d->id);
}

QString SQLPane::group() {
    return PANE_GROUP;
}

QString SQLPane::description() {
    return {};
}

void SQLPane::activation() {

}

void SQLPane::save(bool rename) {

}

void SQLPane::contextMenu(const QPoint& pos) {

}

QJsonObject SQLPane::toJson() {
    QJsonObject data = {
        {"path",this->path()},
        //{"mineType",d->mineType},
        //{"line",ui->editor->firstVisibleBlockNumber()+1}
    };
    return {
        {"id",this->id()},
        {"group",this->group()},
        {"data",data}
    };
}

bool SQLPane::closeEnable() {
    return true;
}

void SQLPane::doAction(int a) {

}

void SQLPane::rename(const QString& name) {

}

void SQLPane::autoSave() {

}

bool SQLPane::readFile(const QString& path) {


    return true;
}

QString SQLPane::path(){
    return {};
}

bool SQLPane::isModified() const{

    return false;
}

CodeEditorView* SQLPane::editor() {
    return nullptr;
}

int SQLPane::fileState() {
    return 0;
}

void SQLPane::setFileState(int state) {

}

void SQLPane::invokeFileState() {

}

bool SQLPane::isModification() {

    return false;
}

void SQLPane::reload() {

}

SQLPane* SQLPane::make(DockingPaneManager* dockingManager,const QJsonObject& data){
    return new SQLPane(dockingManager->widget());
}




}
