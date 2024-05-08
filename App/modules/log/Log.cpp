#include "Log.h"
#include <QDateTime>
namespace ady {
    Log* Log::instance = nullptr;

    Log::Log()
    {


    }

    Log* Log::getInstance()
    {
        if(instance==nullptr){
            instance = new Log();
        }
        return instance;
    }


    QVariant Log::data(const QModelIndex &index, int role) const {
        if (!index.isValid())
            return QVariant();

        if (index.row() >= m_data.size() || index.row() < 0)
            return QVariant();
        if (role == Qt::DisplayRole) {
            const auto &item = m_data.at(index.row());
            if (index.column() == Datetime){
                return item.datetime;
            }else if (index.column() == Site){
                return item.site_name;
            }else if(index.column() == Description){
                return item.description;
            }
        }
        return QVariant();
    }

    QVariant Log::headerData(int section, Qt::Orientation orientation,int role) const
    {
        if (role == Qt::DisplayRole) {
            switch (section) {
                case Datetime:
                    return tr("Datetime");
                case Site:
                    return tr("Site Name");
                case Description:
                    return tr("Description");
                default:
                    return QVariant();
            }
        }
        return QVariant();
    }

    int Log::rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return m_data.size();
    }

    int Log::columnCount(const QModelIndex &parent) const
    {
         Q_UNUSED(parent);
        return Max;
    }

    void Log::setLists(QList<LogItem> data)
    {
        beginResetModel();
        this->m_data = data;
        endResetModel();
    }

    void Log::appendItem(LogItem item)
    {
        int count = this->rowCount();
        beginInsertRows(QModelIndex(),count,count+1);
        this->m_data.append(item);
        endInsertRows();
    }

     void Log::clearAll()
     {
         beginResetModel();
         m_data.clear();
         endResetModel();
     }

     void Log::transfer(const QString& siteName,const QString& filename,LogItem::CMD command)
     {
         QString description;
        if(command==LogItem::UPLOAD){
            description = QObject::tr("Upload %1").arg(filename);
        }else{
            description = QObject::tr("Download %1").arg(filename);
        }
        Log::push(siteName,description,command);
     }

     void Log::push(const QString& siteName,const QString& description,LogItem::CMD command)
     {
        LogItem item;
        item.site_name = siteName;
        item.description = description;
        item.command = command;
        item.datetime = QDateTime::currentDateTime().toString(Qt::ISODate);
        getInstance()->appendItem(item);
     }
}
