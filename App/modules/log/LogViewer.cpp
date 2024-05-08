#include "LogViewer.h"
#include "ui_LogViewer.h"
#include "Log.h"
namespace ady {
    LogViewer::LogViewer(QWidget* parent)
        :QDialog(parent),ui(new Ui::LogViewer()){
        ui->setupUi(this);

        ui->treeView->setModel(Log::getInstance());

        //connect(ui->actionDelete,&QAction::triggered,this,&LogViewer::onDelete);
        connect(ui->actionClear,&QAction::triggered,this,&LogViewer::onClearAll);
    }

    LogViewer::~LogViewer()
    {
        delete ui;
    }

    void LogViewer::scrollBottom()
    {
        ui->treeView->scrollToBottom();
    }

    void LogViewer::onDelete()
    {

    }

    void LogViewer::onClearAll()
    {
        Log::getInstance()->clearAll();
    }

}
