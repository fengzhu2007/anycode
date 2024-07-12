#include "diff_file_model.h"
#include <QIcon>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>
#include "common/utils.h"
namespace ady {

DiffFileModel::DiffFileModel(QObject* parent)
    :QAbstractTableModel(parent)
{
    m_additionIcon.addFile(QString::fromUtf8(":/img/Resource/cvs_file_addition.svg"));
    m_deletionIcon.addFile(QString::fromUtf8(":/img/Resource/cvs_file_deletion.svg"));
    m_changeIcon.addFile(QString::fromUtf8(":/img/Resource/cvs_file_change.svg"));
}

QVariant DiffFileModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_data.size() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole) {
        const auto &item = m_data.at(index.row());
        int column = index.column();
        //QFileInfo fi(m_projectDir + "/" + item.path());
        if(column==Name){
            return item.path();
        }else if(column == Filesize){
            if(/*fi.exists()*/item.filesize()>-1l){
                return Utils::readableFilesize(item.filesize());
            }
        }else if(column == ModifyTime){
            if(/*fi.exists()*/item.filesize()>-1l){
                //return fi.fileTime(QFile::FileTime::FileModificationTime).toString("yyyy-MM-dd HH:mm");
                return item.filetime();
            }
        }
    }else if(role==Qt::DecorationRole){
        const auto &item = m_data.at(index.row());
        if(index.column()==Name){
            cvs::DiffFile::Status status = item.status();
            if(status==cvs::DiffFile::Addition){
                //return QIcon(QString::fromUtf8(":/img/Resource/cvs_file_addition.svg"));
                return m_additionIcon;
            }else if(status==cvs::DiffFile::Deletion){
                //return QIcon(QString::fromUtf8(":/img/Resource/cvs_file_deletion.svg"));
                return m_deletionIcon;
            }else if(status==cvs::DiffFile::Change){
                //return QIcon(QString::fromUtf8(":/img/Resource/cvs_file_change.svg"));
                return m_changeIcon;
            }
        }
    }else if(role==Qt::TextColorRole){
        const auto &item = m_data.at(index.row());
        if(item.status() == cvs::DiffFile::Deletion){
            return QColor(Qt::gray);
        }
    }
    return QVariant();
}

Qt::ItemFlags DiffFileModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    return flags;
}

QVariant DiffFileModel::headerData(int section, Qt::Orientation orientation,int role) const
{
    if (role == Qt::DisplayRole) {
        switch (section) {
            case Name:
                return tr("Name");
            case Filesize:
                return tr("Filesize");
            case ModifyTime:
                return tr("Modify Time");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

int DiffFileModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;
    return m_data.size();
}

int DiffFileModel::columnCount(const QModelIndex &parent) const
{
    return Max;
}

cvs::DiffFile DiffFileModel::getItem(int pos)
{
    return m_data.at(pos);
}

cvs::DiffFile DiffFileModel::at(int row){
    return m_data.at(row);
}

void DiffFileModel::setDataSource(QList<cvs::DiffFile> data){
    beginResetModel();
    m_data = data;
    endResetModel();
}

void DiffFileModel::setList(QList<cvs::DiffFile> data)
{
    beginResetModel();
    m_data = data;
    endResetModel();
}

void DiffFileModel::clear()
{
    beginResetModel();
    m_data.clear();
    endResetModel();
}

}
