#ifndef TEMPLATE_COMMAND_HISTORY_H
#define TEMPLATE_COMMAND_HISTORY_H

#include <QString>
#include <QSqlQuery>
#include <QList>
#include "storage.h"
#include "global.h"

namespace ady {

    class ANYENGINE_EXPORT TemplateCommandHistoryRecord {
    public:
        long long id = 0;
        QString template_title;   // template name
        QString command_template; // original template with %1, %2 placeholders
        QString command;          // resolved command (placeholders replaced)
        QString params;           // JSON array of parameter values
        QString param_types;      // comma-separated param type ids, e.g. "0,2,3"
        QString shell_type;       // "cmd" or "powershell"
        QString created_at;       // ISO timestamp
    };

    class ANYENGINE_EXPORT TemplateCommandHistoryStorage : public Storage {
    public:
        constexpr const static char TABLE_NAME[] = "template_command_history";
        constexpr const static char COL_TEMPLATE_TITLE[] = "template_title";
        constexpr const static char COL_COMMAND_TEMPLATE[] = "command_template";
        constexpr const static char COL_COMMAND[] = "command";
        constexpr const static char COL_PARAMS[] = "params";
        constexpr const static char COL_PARAM_TYPES[] = "param_types";
        constexpr const static char COL_SHELL_TYPE[] = "shell_type";
        constexpr const static char COL_CREATED_AT[] = "created_at";

        TemplateCommandHistoryStorage();

        QList<TemplateCommandHistoryRecord> all();
        long long insert(TemplateCommandHistoryRecord record);
        bool updateTimestamp(long long id);
        long long findDuplicate(const QString& command, const QString& params);
        bool del(long long id);
        bool trimTo(int maxCount);

    private:
        TemplateCommandHistoryRecord toRecord(QSqlQuery& query);
    };

}

#endif // TEMPLATE_COMMAND_HISTORY_H
