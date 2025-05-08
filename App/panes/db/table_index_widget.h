#ifndef TABLE_INDEX_WIDGET_H
#define TABLE_INDEX_WIDGET_H

#include <QWidget>

namespace Ui {
class TableIndexWidget;
}

namespace ady{
class TableIndex;
class TableIndexWidgetPrivate;
class TableIndexWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TableIndexWidget(long long id,const QString& table,QWidget *parent = nullptr);
    ~TableIndexWidget();
    void initData();
    void setTableName(const QString& tableName);

public slots:
    void onFieldActivated(const QModelIndex& index);
    void onActionTriggered();
    void onIndexChanged(const QString& name,TableIndex* index);
    void onFieldsChanged(const QStringList& fields);

private:
    Ui::TableIndexWidget *ui;
    TableIndexWidgetPrivate* d;
};
}
#endif // TABLE_INDEX_WIDGET_H
