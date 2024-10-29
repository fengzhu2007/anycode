#include "remote_file_item_model.h"
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QMimeData>
#include "common/utils.h"
#include "common.h"
namespace ady {

class FileItemSorting final{
public:
    FileItemSorting(RemoteFileItemModel::Column col,bool asc){
        this->col = col;
        this->asc = asc;
    }
    bool operator()(FileItem& a,FileItem& b)
    {
        if(this->asc){
            if(a.type!=b.type){
                return a.type>b.type;
            }
            if(this->col==RemoteFileItemModel::Name){
                return a.name.compare(b.name,Qt::CaseInsensitive)<0;
                //return a.name.toLower() < b.name.toLower();
            }else if(this->col==RemoteFileItemModel::Size){
                if(a.type==0){
                    //file
                    return a.size<b.size;
                }else{
                    //dir
                    //return a.name.toLower() < b.name.toLower();
                    return a.name.compare(b.name,Qt::CaseInsensitive)<0;
                }
            }else if(this->col==RemoteFileItemModel::ModifyTime){
                //return a.modifyTime<b.modifyTime;
                return a.modifyTime.compare(b.modifyTime,Qt::CaseInsensitive)<0;
            }else if(this->col==RemoteFileItemModel::Permission){
                //return a.permission<b.permission;
                return a.permission.compare(b.permission,Qt::CaseInsensitive)<0;
            }

        }else{
            if(a.type!=b.type){
                return a.type<b.type;
            }
            if(this->col==RemoteFileItemModel::Name){
                return a.name.compare(b.name,Qt::CaseInsensitive)>=0;
                //return a.name.toLower() < b.name.toLower();
             }else if(this->col==RemoteFileItemModel::Size){
                if(a.type==0){
                    //file
                    return a.size>b.size;
                }else{
                    //dir
                    //return a.name.toLower() < b.name.toLower();
                    return a.name.compare(b.name,Qt::CaseInsensitive)>=0;
                }
            }else if(this->col==RemoteFileItemModel::ModifyTime){
                //return a.modifyTime>b.modifyTime;
                return a.modifyTime.compare(b.modifyTime,Qt::CaseInsensitive)>=0;
            }else if(this->col==RemoteFileItemModel::Permission){
                //return a.permission>b.permission;
                return a.permission.compare(b.permission,Qt::CaseInsensitive)>=0;
            }
        }
        return true;
    }

    void setCol(int col)
    {
        this->col = col;
    }
    void setAsc(bool asc){
        this->asc = asc;
    }
private:
    bool asc;
    int col;
};


RemoteFileItemModel::RemoteFileItemModel(long long id,QObject *parent)
    :QAbstractTableModel(parent)
{
    m_iconProvider = new QFileIconProvider;
    m_sorting = new FileItemSorting(Name,true);
    this->id = id;
}

RemoteFileItemModel::~RemoteFileItemModel()
{
    delete m_iconProvider;
}

QVariant RemoteFileItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_data.size() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole) {
        const auto &item = m_data.at(index.row());
        if (index.column() == Name){
            return item.name;
        }else if (index.column() == Size){
            //return item.size;
            if(item.type==FileItem::Dir){
                return "";
            }else{
                return Utils::readableFilesize(item.size);
            }
        }else if (index.column() == ModifyTime){
            return item.modifyTime;
        }else if(index.column() == Permission){
            return item.permission;
        }

    }else if(role==Qt::DecorationRole){
        const auto &item = m_data.at(index.row());
        if(index.column()==Name){
            if(item.type == FileItem::Dir){
                return m_iconProvider->icon(QFileIconProvider::Folder);
            }else{
                return m_iconProvider->icon(QFileInfo(item.name));
            }

        }
    }else if(role==Qt::EditRole){
        const auto &item = m_data.at(index.row());
        if (index.column() == Name){
            return item.name;
        }
    }
    return QVariant();
}

bool RemoteFileItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role==Qt::EditRole){
        if(index.column() ==Name ){
            int row = index.row();
            QString name = value.toString();
            FileItem item = m_data.at(row);
            if(item.name != name){
                emit cellEditing(index,name);
            }
            return true;
        }
    }
    return QAbstractTableModel::setData(index,value,role);
}

Qt::ItemFlags RemoteFileItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    flags = flags | Qt::ItemIsDragEnabled;
    if(index.column()==Name){
        FileItem item = m_data.at(index.row());
        if(item.enabled && item.name!=".."){
            flags |= Qt::ItemIsEditable;
        }
    }
    return flags;
}

QMimeData *RemoteFileItemModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData* data = new QMimeData;
    int size = indexes.size();
    QList<FileItem>* lists = new QList<FileItem>();
    for(int i=0;i<size;i++){
        QModelIndex index = indexes[i];
        int column = index.column();
        if(column==0){
            int row = index.row();
            if(row<m_data.size()){
               FileItem item = m_data.at(row);
               //lists.push_back(item.path);
               lists->push_back(item);
            }
        }
    }
    if(lists->size()>0){
        QByteArray b;
        QDataStream in(&b,QIODevice::WriteOnly);
        in<<this->id;
        in<<(qint64)lists;
        data->setData(MM_DOWNLOAD,b);
    }else{
        delete lists;
    }
    return data;
}


QVariant RemoteFileItemModel::headerData(int section, Qt::Orientation orientation,int role) const
{
    if (role == Qt::DisplayRole) {
        switch (section) {
            case Name:
                return tr("Name");
            case Size:
                return tr("Size");
            case ModifyTime:
                return tr("Modify Time");
            case Permission:
                return tr("Permission");
            default:
                return QVariant();
        }
    }
    return QVariant();
}


int RemoteFileItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    return m_data.size();
}

int RemoteFileItemModel::columnCount(const QModelIndex &parent) const
{
    return Max;
}

/*QMimeData* FileItemModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData* data = QAbstractItemModel::mimeData(indexes);

}*/


void RemoteFileItemModel::updateAll(const QList<FileItem>& data)
{
    QMutexLocker locker(&mutex);
}

void RemoteFileItemModel::removeItem(FileItem item)
{
    QMutexLocker locker(&mutex);
}

void RemoteFileItemModel::insertItem(int index,FileItem)
{
    QMutexLocker locker(&mutex);
}

void RemoteFileItemModel::updateItem(int index,FileItem)
{
    QMutexLocker locker(&mutex);
}

FileItem RemoteFileItemModel::getItem(int row)
{
    QMutexLocker locker(&mutex);
    return m_data.at(row);
}

void RemoteFileItemModel::setList(QList<FileItem>& data)
{
    QMutexLocker locker(&mutex);
    beginResetModel();
    if(data.size()>0){
        QList<FileItem>::iterator iter = data.begin();
        if((*iter).name.compare("..")==0){
            iter++;
        }
        qSort(iter,data.end(),*m_sorting);
    }
    m_data = data;
    endResetModel();
}


void RemoteFileItemModel::sortList(int col, Qt::SortOrder order)
{
    QMutexLocker locker(&mutex);
    m_sorting->setCol(col);
    m_sorting->setAsc(order==0);
    beginResetModel();
    if(m_data.size()>0){
        QList<FileItem>::iterator iter = m_data.begin();
        if((*iter).name.compare("..")==0){
            iter++;
        }
        qSort(iter,m_data.end(),*m_sorting);
    }
    endResetModel();
}



}
