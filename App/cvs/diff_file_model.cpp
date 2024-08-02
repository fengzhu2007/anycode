#include "diff_file_model.h"
#include <QIcon>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>
#include "common/utils.h"



namespace ady {


class DiffFileModelPrivate{
public:
    QList<cvs::DiffFile> data;
    QString projectDir;
    QIcon additionIcon;
    QIcon deletionIcon;
    QIcon changeIcon;
};



DiffFileModel::DiffFileModel(QObject* parent)
    :QAbstractTableModel(parent)
{
    d = new DiffFileModelPrivate;
    d->additionIcon.addFile(QString::fromUtf8(":/img/Resource/cvs_file_addition.svg"));
    d->deletionIcon.addFile(QString::fromUtf8(":/img/Resource/cvs_file_deletion.svg"));
    d->changeIcon.addFile(QString::fromUtf8(":/img/Resource/cvs_file_change.svg"));
}

DiffFileModel::~DiffFileModel(){
    delete d;
}

QVariant DiffFileModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= d->data.size() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole) {
        const auto &item = d->data.at(index.row());
        int column = index.column();
        //QFileInfo fi(m_projectDir + "/" + item.path());
        if(column==Name){
            return item.path();
        }/*else if(column == Filesize){
            if(item.filesize()>-1l){
                return Utils::readableFilesize(item.filesize());
            }
        }*/else if(column == ModifyTime){
            if(/*fi.exists()*/item.filesize()>-1l){
                return item.filetime().toString("MM-dd HH:mm");
                //return item.filetime();
            }
        }
    }else if(role == Qt::ToolTipRole){
        const auto &item = d->data.at(index.row());
        long long filesize = item.filesize();
        if(filesize==-1l){
            return tr("File Name:%1").arg(item.path());
        }else{
            return tr("File Name:%1\nFile Size:%2\nLast Modify:%3").arg(item.path()).arg(Utils::readableFilesize(item.filesize())).arg(item.filetime().toString("yyyy-MM-dd HH:mm"));
        }

        //return tr("File Name:%1\nFile Size:%2\nLast Modify:%3").arg(item.path()).arg(item.filesize()>-1l?Utils::readableFilesize(item.filesize()):"").arg()
        //return tr("ID:%1\nDate:%2\nAuthor:%3\nMessage:%4").arg(item.oid()).arg(item.time().toString("yyyy-MM-dd HH:mm")).arg(item.author()).arg(item.content());
    }else if(role==Qt::DecorationRole){
        const auto &item = d->data.at(index.row());
        if(index.column()==Name){
            cvs::DiffFile::Status status = item.status();
            if(status==cvs::DiffFile::Addition){
                //return QIcon(QString::fromUtf8(":/img/Resource/cvs_file_addition.svg"));
                return d->additionIcon;
            }else if(status==cvs::DiffFile::Deletion){
                //return QIcon(QString::fromUtf8(":/img/Resource/cvs_file_deletion.svg"));
                return d->deletionIcon;
            }else if(status==cvs::DiffFile::Change){
                //return QIcon(QString::fromUtf8(":/img/Resource/cvs_file_change.svg"));
                return d->changeIcon;
            }
        }
    }else if(role==Qt::ForegroundRole){
        const auto &item = d->data.at(index.row());
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
            /*case Filesize:
                return tr("Filesize");*/
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
    return d->data.size();
}

int DiffFileModel::columnCount(const QModelIndex &parent) const
{
    return Max;
}

cvs::DiffFile DiffFileModel::getItem(int pos)
{
    return d->data.at(pos);
}

cvs::DiffFile DiffFileModel::at(int row){
    return d->data.at(row);
}

void DiffFileModel::setDataSource(QList<cvs::DiffFile> data){
    beginResetModel();
    d->data = data;
    endResetModel();
}

void DiffFileModel::setList(QList<cvs::DiffFile> data)
{
    beginResetModel();
    d->data = data;
    endResetModel();
}

void DiffFileModel::clear()
{
    beginResetModel();
    d->data.clear();
    endResetModel();
}

void DiffFileModel::setProjectDir(QString dir){
    d->projectDir = dir;
}
QList<cvs::DiffFile> DiffFileModel::lists(){
    return d->data;
}
}
