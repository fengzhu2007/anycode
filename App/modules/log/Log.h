#ifndef LOG_H
#define LOG_H
#include <QAbstractListModel>
#include <QList>
#include "LogItem.h"
namespace ady {
    class Log : public QAbstractListModel
    {
        Q_OBJECT
    private:
        Log();

    public:
        enum Column {
            Datetime=0,
            Site,
            Description,
            Max
        };

        static Log* getInstance();

        QVariant data(const QModelIndex &index, int role) const override;
        QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        void setLists(QList<LogItem> data);
        void appendItem(LogItem item);
        void clearAll();


        static void transfer(const QString& siteName,const QString& filename,LogItem::CMD command);
        static void push(const QString& siteName,const QString& description,LogItem::CMD command);

    private:
        static Log* instance;
        QList<LogItem> m_data;

    };
}

#endif // LOG_H
