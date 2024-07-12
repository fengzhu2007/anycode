#include "FileItemModel.h"
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QMimeData>
#include <QDataStream>
#include "common/utils.h"
#include "common.h"
namespace ady {

class LocalFileItemSorting final{
public:
    LocalFileItemSorting(FileItemModel::Column col,bool asc){
        this->col = col;
        this->asc = asc;
    }
    bool operator()(FileItem& a,FileItem& b)
    {
        if(this->asc){
            if(a.type!=b.type){
                return a.type>b.type;
            }
            if(this->col==FileItemModel::Name){
                return a.name.compare(b.name,Qt::CaseInsensitive)<0;
                //return a.name.toLower() < b.name.toLower();
            }else if(this->col==FileItemModel::Size){
                if(a.type==0){
                    //file
                    return a.size<b.size;
                }else{
                    //dir
                    //return a.name.toLower() < b.name.toLower();
                    return a.name.compare(b.name,Qt::CaseInsensitive)<0;
                }
            }else if(this->col==FileItemModel::ModifyTime){
                //return a.modifyTime<b.modifyTime;
                return a.modifyTime.compare(b.modifyTime,Qt::CaseInsensitive)<0;
            }

        }else{
            if(a.type!=b.type){
                return a.type<b.type;
            }
            if(this->col==FileItemModel::Name){
                return a.name.compare(b.name,Qt::CaseInsensitive)>=0;
                //return a.name.toLower() < b.name.toLower();
             }else if(this->col==FileItemModel::Size){
                if(a.type==0){
                    //file
                    return a.size>b.size;
                }else{
                    //dir
                    //return a.name.toLower() < b.name.toLower();
                    return a.name.compare(b.name,Qt::CaseInsensitive)>=0;
                }
            }else if(this->col==FileItemModel::ModifyTime){
                //return a.modifyTime>b.modifyTime;
                return a.modifyTime.compare(b.modifyTime,Qt::CaseInsensitive)>=0;
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



FileItemModel::FileItemModel(QObject *parent)
    :QAbstractTableModel(parent)
{
    m_iconProvider = new QFileIconProvider;
    m_sorting = new LocalFileItemSorting(Name,true);
}

FileItemModel::~FileItemModel()
{
    delete m_iconProvider;
}

QVariant FileItemModel::data(const QModelIndex &index, int role) const
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
        }

    }else if(role==Qt::DecorationRole){
        const auto &item = m_data.at(index.row());
        if(index.column()==Name){
            if(item.name==".."){
                return m_iconProvider->icon(QFileIconProvider::Folder);
            }else{
                return m_iconProvider->icon(QFileInfo(item.path));
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

bool FileItemModel::setData(const QModelIndex &index, const QVariant &value, int role )
{
    if(role==Qt::EditRole){
        if(index.column() ==Name ){
            int row = index.row();
            QString name = value.toString();
            FileItem item = m_data.at(row);
            QFileInfo fi(item.path);
            if(fi.exists() && fi.fileName()!=name){
                QString fileName = fi.path() + "/" + name;
                QDir d(fi.path());
                bool ret = d.rename(fi.fileName(),name);
                if(ret){
                    item.name = name;
                    item.path = fileName;
                    m_data[row] = item;
                }
                return ret;
            }else{
                return false;
            }
        }
    }
    return QAbstractTableModel::setData(index,value,role);
}

Qt::ItemFlags FileItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    flags = flags | Qt::ItemIsDragEnabled ;
    if(index.column()==Name){
        FileItem item = m_data.at(index.row());
        if(item.enabled && item.name!=".."){
            flags |= Qt::ItemIsEditable;
        }
    }

    return flags;
}

QVariant FileItemModel::headerData(int section, Qt::Orientation orientation,int role) const
{
    if (role == Qt::DisplayRole) {
        switch (section) {
            case Name:
                return tr("Name");
            case Size:
                return tr("Size");
            case ModifyTime:
                return tr("Modify Time");
            default:
                return QVariant();
        }
    }
    return QVariant();
}


int FileItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    return m_data.size();
}

int FileItemModel::columnCount(const QModelIndex &parent) const
{
    return Max;
}

/*QMimeData* FileItemModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData* data = QAbstractItemModel::mimeData(indexes);

}*/


QMimeData* FileItemModel::mimeData(const QModelIndexList &indexes) const
{
    //QMimeData* data = QAbstractItemModel::mimeData(indexes);
    QMimeData* data = new QMimeData;
    int size = indexes.size();
    //QStringList lists;
    QList<FileItem>* lists = new QList<FileItem>();
    for(int i=0;i<size;i++){
        QModelIndex index = indexes[i];
        int column = index.column();
        if(column==0){
            int row = index.row();
            if(row<m_data.size()){
               FileItem item = m_data.at(row);
               lists->push_back(item);
            }
        }
    }
    if(lists->size()>0){
        QByteArray b;
        QDataStream in(&b,QIODevice::WriteOnly);
        in<<(qint64)lists;
        data->setData(MM_UPLOAD,b);
    }else{
        delete lists;
    }
    return data;
}




void FileItemModel::updateAll(QList<FileItem> data)
{

}

void FileItemModel::removeItem(FileItem item)
{

}

void FileItemModel::insertItem(int index,FileItem)
{

}

void FileItemModel::updateItem(int index,FileItem)
{

}

FileItem FileItemModel::getItem(int row)
{
    return m_data.at(row);
}

void FileItemModel::readDir(const QString& dir)
{
    beginResetModel();
    m_data.clear();
    QDir d(dir);
    QFileInfoList lists = d.entryInfoList(QDir::Files|QDir::Dirs,QDir::Name|QDir::DirsFirst);
    int length = lists.length();
    for(int i=0;i<length;i++){
        QFileInfo fi = lists.at(i);
        if(fi.fileName()=="."){
            continue;
        }

        if(fi.fileName()==".."){
            continue;
            //item.type = FileItem::Dir;
            //qDebug()<<"readDir:"<<fi.filePath();
        }
        FileItem item;
        item.type = fi.isDir()?FileItem::Dir:FileItem::File;
        item.name = fi.fileName();
        item.path = fi.filePath();
        item.size = fi.size();
        item.modifyTime = fi.fileTime(QFileDevice::FileModificationTime).toString("yyyy-MM-dd HH:mm");
        m_data.push_back(item);
    }
    FileItem item;
    item.type = FileItem::Dir;
    item.name = "..";
    item.path = dir  + (dir.endsWith("/")?"":"/")+ item.name;
    //qDebug()<<"readDir:"<<item.path;
    item.size = 0;
    item.modifyTime = "";
    m_data.insert(0,item);
    endResetModel();
}

void FileItemModel::setList(QList<FileItem> data)
{

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


void FileItemModel::sortList(int col, Qt::SortOrder order)
{
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
