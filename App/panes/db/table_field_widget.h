#ifndef TABLE_FIELD_WIDGET_H
#define TABLE_FIELD_WIDGET_H

#include <QWidget>

namespace Ui {
class TableFieldWidget;
}

namespace ady{
class TableField;
class TableFieldWidgetPrivate;
class TableFieldWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TableFieldWidget(long long id,const QString& table,QWidget *parent = nullptr);
    ~TableFieldWidget();

    void initData();
    void setTableName(const QString& tableName);


    void notifyFieldsChanged();

signals:
    void fieldsChanged(const QStringList& fields);

public slots:
    void onFieldActivated(const QModelIndex& index);
    void onActionTriggered();
    void onFieldChanged(const QString& name,TableField* field);



private:
    Ui::TableFieldWidget *ui;
    TableFieldWidgetPrivate* d;
};
}

#endif // TABLE_FIELD_WIDGET_H
