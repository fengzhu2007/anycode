#include "template_command_storage.h"
#include "database_helper.h"
#include <QVariant>
#include <QDebug>

namespace ady {

constexpr const char TemplateCommandStorage::TABLE_NAME[];
constexpr const char TemplateCommandStorage::COL_TITLE[];
constexpr const char TemplateCommandStorage::COL_COMMAND_TEMPLATE[];
constexpr const char TemplateCommandStorage::COL_PARAM_TYPES[];
constexpr const char TemplateCommandStorage::COL_DEFAULT_VALUES[];
constexpr const char TemplateCommandStorage::COL_SHELL_TYPE[];
constexpr const char TemplateCommandStorage::COL_LISTORDER[];

TemplateCommandStorage::TemplateCommandStorage()
{

}

TemplateCommandRecord TemplateCommandStorage::one(long long id)
{
    TemplateCommandRecord record;
    QString sql = QString("SELECT * FROM [%1] WHERE [%2]=?")
                      .arg(TABLE_NAME).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0, id);
    bool ret = query.exec();
    this->error = query.lastError();
    if (ret && query.next()) {
        record = toRecord(query);
    } else {
        record.id = 0;
    }
    return record;
}

QList<TemplateCommandRecord> TemplateCommandStorage::all()
{
    QList<TemplateCommandRecord> lists;
    QString sql = QString("SELECT * FROM [%1] WHERE 1 ORDER BY [%2] ASC")
                      .arg(TABLE_NAME).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    bool ret = query.exec(sql);
    this->error = query.lastError();
    if (ret) {
        while (query.next()) {
            lists.push_back(toRecord(query));
        }
    }
    return lists;
}

long long TemplateCommandStorage::insert(TemplateCommandRecord record)
{
    QString sql = QString("INSERT INTO [%1] ([%2],[%3],[%4],[%5],[%6],[%7]) VALUES (?,?,?,?,?,?)")
                      .arg(TABLE_NAME)
                      .arg(COL_TITLE)
                      .arg(COL_COMMAND_TEMPLATE)
                      .arg(COL_PARAM_TYPES)
                      .arg(COL_DEFAULT_VALUES)
                      .arg(COL_SHELL_TYPE)
                      .arg(COL_LISTORDER);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0, record.title);
    query.bindValue(1, record.command_template);
    query.bindValue(2, record.param_types);
    query.bindValue(3, record.default_values);
    query.bindValue(4, record.shell_type);
    query.bindValue(5, record.listorder);
    bool ret = query.exec();
    this->error = query.lastError();
    if (ret) {
        return query.lastInsertId().toLongLong();
    } else {
        return 0;
    }
}

bool TemplateCommandStorage::update(TemplateCommandRecord record)
{
    QString sql = QString("UPDATE [%1] SET [%2]=?,[%3]=?,[%4]=?,[%5]=?,[%6]=?,[%7]=? WHERE [%8]=?")
                      .arg(TABLE_NAME)
                      .arg(COL_TITLE)
                      .arg(COL_COMMAND_TEMPLATE)
                      .arg(COL_PARAM_TYPES)
                      .arg(COL_DEFAULT_VALUES)
                      .arg(COL_SHELL_TYPE)
                      .arg(COL_LISTORDER)
                      .arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0, record.title);
    query.bindValue(1, record.command_template);
    query.bindValue(2, record.param_types);
    query.bindValue(3, record.default_values);
    query.bindValue(4, record.shell_type);
    query.bindValue(5, record.listorder);
    query.bindValue(6, record.id);
    bool ret = query.exec();
    this->error = query.lastError();
    return ret;
}

bool TemplateCommandStorage::del(long long id)
{
    QString sql = QString("DELETE FROM [%1] WHERE [%2]=?")
                      .arg(TABLE_NAME).arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0, id);
    bool ret = query.exec();
    this->error = query.lastError();
    return ret;
}

TemplateCommandRecord TemplateCommandStorage::toRecord(QSqlQuery& query)
{
    TemplateCommandRecord record;
    record.id = query.value(DatabaseHelper::COL_ID).toLongLong();
    record.title = query.value(COL_TITLE).toString();
    record.command_template = query.value(COL_COMMAND_TEMPLATE).toString();
    record.param_types = query.value(COL_PARAM_TYPES).toString();
    record.default_values = query.value(COL_DEFAULT_VALUES).toString();
    record.shell_type = query.value(COL_SHELL_TYPE).toString();
    record.listorder = query.value(COL_LISTORDER).toInt();
    return record;
}

}
