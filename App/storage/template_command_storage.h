#ifndef TEMPLATE_COMMAND_STORAGE_H
#define TEMPLATE_COMMAND_STORAGE_H

#include <QString>
#include <QSqlQuery>
#include <QList>
#include "storage.h"
#include "global.h"

namespace ady {

    class ANYENGINE_EXPORT TemplateCommandRecord {
    public:
        long long id = 0;
        QString title;
        QString command_template;
        QString param_types;   // comma-separated integers, e.g. "0,1,2"
        QString default_values; // comma-separated default values, e.g. "val1,val2"
        QString shell_type;    // "cmd" or "powershell"
        int listorder = 0;
    };

    class ANYENGINE_EXPORT TemplateCommandStorage : public Storage {
    public:
        constexpr const static char TABLE_NAME[] = "template_command";
        constexpr const static char COL_TITLE[] = "title";
        constexpr const static char COL_COMMAND_TEMPLATE[] = "command_template";
        constexpr const static char COL_PARAM_TYPES[] = "param_types";
        constexpr const static char COL_DEFAULT_VALUES[] = "default_values";
        constexpr const static char COL_SHELL_TYPE[] = "shell_type";
        constexpr const static char COL_LISTORDER[] = "listorder";

        TemplateCommandStorage();

        TemplateCommandRecord one(long long id);
        QList<TemplateCommandRecord> all();
        long long insert(TemplateCommandRecord record);
        bool update(TemplateCommandRecord record);
        bool del(long long id);

    private:
        TemplateCommandRecord toRecord(QSqlQuery& query);
    };

}
#endif // TEMPLATE_COMMAND_STORAGE_H
