#include "table_field_widget.h"

#include <QToolBar>
#include <QSplitter>
#include <QTreeView>


namespace Ui {
class TableFieldWidget{
public :
    QToolBar* toolBar;
    QSplitter* splitter;
    QTreeView* treeView;



    void setupUi(QWidget* parent){



    }
};
}



namespace  ady {

TableFieldWidget::TableFieldWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TableFieldWidget)
{
    ui->setupUi(this);
}

TableFieldWidget::~TableFieldWidget()
{
    delete ui;
}
}

