#include "table_scheme_widget.h"
#include "ui_table_scheme_widget.h"
#include "table_field_widget.h"
#include "table_index_widget.h"
#include "table_option_widget.h"


namespace ady{
class TableSchemeWidgetPrivate{
public:
    long long id;
    QString name;
    TableFieldWidget* fieldTab;
    TableIndexWidget* indexTab;
    TableOptionWidget* optionTab;



};

TableSchemeWidget::TableSchemeWidget(long long id,const QString& table,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TableSchemeWidget)
{
    ui->setupUi(this);
    d = new TableSchemeWidgetPrivate;
    d->id = id;
    d->name = table;

    d->fieldTab = new TableFieldWidget(id,table,ui->tabWidget);
    d->indexTab = new TableIndexWidget(id,table,ui->tabWidget);
    d->optionTab = new TableOptionWidget(id,table,ui->tabWidget);

    ui->tabWidget->addTab(d->fieldTab,tr("Field"));
    ui->tabWidget->addTab(d->indexTab,tr("Index"));
    ui->tabWidget->addTab(d->optionTab,tr("Option"));

}

TableSchemeWidget::~TableSchemeWidget()
{
    delete ui;
    delete d;
}

}
