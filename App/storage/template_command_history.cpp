#include "template_command_history.h"
#include "database_helper.h"
#include <QVariant>
#include <QDateTime>

namespace ady {

constexpr const char TemplateCommandHistoryStorage::TABLE_NAME[];
constexpr const char TemplateCommandHistoryStorage::COL_TEMPLATE_TITLE[];
constexpr const char TemplateCommandHistoryStorage::COL_COMMAND_TEMPLATE[];
constexpr const char TemplateCommandHistoryStorage::COL_COMMAND[];
constexpr const char TemplateCommandHistoryStorage::COL_PARAMS[];
constexpr const char TemplateCommandHistoryStorage::COL_PARAM_TYPES[];
constexpr const char TemplateCommandHistoryStorage::COL_SHELL_TYPE[];
constexpr const char TemplateCommandHistoryStorage::COL_CREATED_AT[];

TemplateCommandHistoryStorage::TemplateCommandHistoryStorage()
{
}

QList<TemplateCommandHistoryRecord> TemplateCommandHistoryStorage::all()
{
    QList<TemplateCommandHistoryRecord> lists;
    QString sql = QString("SELECT * FROM [%1] WHERE 1 ORDER BY [%2] DESC")
                      .arg(TABLE_NAME).arg(COL_CREATED_AT);
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

long long TemplateCommandHistoryStorage::insert(TemplateCommandHistoryRecord record)
{
    QString sql = QString("INSERT INTO [%1] ([%2],[%3],[%4],[%5],[%6],[%7],[%8]) VALUES (?,?,?,?,?,?,?)")
                      .arg(TABLE_NAME)
                      .arg(COL_TEMPLATE_TITLE)
                      .arg(COL_COMMAND_TEMPLATE)
                      .arg(COL_COMMAND)
                      .arg(COL_PARAMS)
                      .arg(COL_PARAM_TYPES)
                      .arg(COL_SHELL_TYPE)
                      .arg(COL_CREATED_AT);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0, record.template_title);
    query.bindValue(1, record.command_template);
    query.bindValue(2, record.command);
    query.bindValue(3, record.params);
    query.bindValue(4, record.param_types);
    query.bindValue(5, record.shell_type);
    query.bindValue(6, record.created_at);
    bool ret = query.exec();
    this->error = query.lastError();
    if (ret) {
        return query.lastInsertId().toLongLong();
    }
    return 0;
}

bool TemplateCommandHistoryStorage::updateTimestamp(long long id)
{
    QString sql = QString("UPDATE [%1] SET [%2]=? WHERE [%3]=?")
                      .arg(TABLE_NAME)
                      .arg(COL_CREATED_AT)
                      .arg(DatabaseHelper::COL_ID);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0, QDateTime::currentDateTime().toString(Qt::ISODate));
    query.bindValue(1, id);
    bool ret = query.exec();
    this->error = query.lastError();
    return ret;
}

long long TemplateCommandHistoryStorage::findDuplicate(const QString& command, const QString& params)
{
    QString sql = QString("SELECT [%1] FROM [%2] WHERE [%3]=? AND [%4]=?")
                      .arg(DatabaseHelper::COL_ID)
                      .arg(TABLE_NAME)
                      .arg(COL_COMMAND)
                      .arg(COL_PARAMS);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    query.prepare(sql);
    query.bindValue(0, command);
    query.bindValue(1, params);
    if (query.exec() && query.next()) {
        return query.value(0).toLongLong();
    }
    return 0;
}

bool TemplateCommandHistoryStorage::del(long long id)
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

bool TemplateCommandHistoryStorage::trimTo(int maxCount)
{
    QString sql = QString("DELETE FROM [%1] WHERE [%2] NOT IN ("
                          "SELECT [%2] FROM [%1] ORDER BY [%3] DESC LIMIT %4)")
                      .arg(TABLE_NAME)
                      .arg(DatabaseHelper::COL_ID)
                      .arg(COL_CREATED_AT)
                      .arg(maxCount);
    QSqlQuery query(DatabaseHelper::getDatabase()->get());
    bool ret = query.exec(sql);
    this->error = query.lastError();
    return ret;
}

TemplateCommandHistoryRecord TemplateCommandHistoryStorage::toRecord(QSqlQuery& query)
{
    TemplateCommandHistoryRecord record;
    record.id = query.value(DatabaseHelper::COL_ID).toLongLong();
    record.template_title = query.value(COL_TEMPLATE_TITLE).toString();
    record.command_template = query.value(COL_COMMAND_TEMPLATE).toString();
    record.command = query.value(COL_COMMAND).toString();
    record.params = query.value(COL_PARAMS).toString();
    record.param_types = query.value(COL_PARAM_TYPES).toString();
    record.shell_type = query.value(COL_SHELL_TYPE).toString();
    record.created_at = query.value(COL_CREATED_AT).toString();
    return record;
}

}
