#ifndef DATAIMPORT_H
#define DATAIMPORT_H
#include <QXmlStreamReader>


#include "storage/ProjectStorage.h"
#include "storage/GroupStorage.h"
#include "storage/SiteStorage.h"

namespace ady {
    class DataImportGroup{
    public:
        GroupRecord group;
        QList<SiteRecord> sites;
    };


    class DataImportProject{
    public:
        ProjectRecord project;
        QList<DataImportGroup> groups;
    };




    class DataImport
    {

    public:
        DataImport();
        ~DataImport();
        int doImport(QWidget* parent);


    private:
        bool readBaseInfo();
        bool readData();
        bool readProjects();
        bool readProject();


        bool readGroups(QList<DataImportGroup>& lists);
        bool readSites(QList<SiteRecord>& lists);

        int importData();


    private:
        QString name;
        QString creator;
        int version = 0;
        QXmlStreamReader* pReader;
        QList<DataImportProject> data;
    };
}
#endif // DATAIMPORT_H