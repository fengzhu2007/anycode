#include "dir_mapping_view.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QApplication>
#include <QMouseEvent>
#include <QPushButton>
#include <QHBoxLayout>
#include <QWidget>
#include <QResizeEvent>
#include <QToolButton>
#include <QPen>
#include <QPainter>
#include <QDebug>
namespace ady {

    DirMappingDelegate::DirMappingDelegate(QObject* parent)
        :QStyledItemDelegate(parent)
    {
        m_gridPen.setStyle(Qt::SolidLine);
        m_gridPen.setColor(Qt::lightGray);
    }


    void DirMappingDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyledItemDelegate::paint(painter, option, index);
        QPen oldPen = painter->pen();
        painter->setPen(m_gridPen);
        painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
        painter->setPen(oldPen);
    }

    /*****************DirMappingModel*****************/


    DirMappingModel::DirMappingModel(QObject* parent)
        :QAbstractTableModel(parent)
    {

    }

    QVariant DirMappingModel::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant();
        if (index.row() >= m_data.size() || index.row() < 0)
            return QVariant();
        if (role == Qt::DisplayRole || role==Qt::EditRole ) {
            int row = index.row();
            const auto &item = m_data.at(row);
            if (index.column() == Src){
                return item.src;
            }else if (index.column() == Dst){
                return item.dst;
            }
        }
        return QVariant();
    }

    bool DirMappingModel::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        if(role==Qt::EditRole){
            int row = index.row();
            auto item = m_data.at(row);
            if(index.column() ==Src ){
                item.src = value.toString();
                m_data[row] = item;
            }else if(index.column() == Dst){
                item.dst = value.toString();
                m_data[row] = item;
            }
        }
        return QAbstractTableModel::setData(index,value,role);
    }

    Qt::ItemFlags DirMappingModel::flags(const QModelIndex &index) const
    {
        if (!index.isValid())
            return 0;
        Qt::ItemFlags flags = QAbstractItemModel::flags(index);
        //flags = flags | Qt::ItemIsDragEnabled ;
        int row = index.row();
        int size = m_data.size();
        int col = index.column();
        if((col==Src || col == Dst) ){
            flags |= Qt::ItemIsEditable;
        }
        return flags;
    }

    QVariant DirMappingModel::headerData(int section, Qt::Orientation orientation,int role) const
    {
        if (role == Qt::DisplayRole) {
            return m_header.size() > section ? m_header.at(section):QVariant();
        }
        return QVariant();
    }

    int DirMappingModel::rowCount(const QModelIndex &parent) const
    {
        return m_data.size() ;
    }

    int DirMappingModel::columnCount(const QModelIndex &parent) const
    {
        int size = m_header.size();
        return size>Max?Max:size;
    }

    void DirMappingModel::setLists(QList<DirMappingItem> data)
    {
        beginResetModel();
        m_data = data;
        endResetModel();
    }

    DirMappingItem DirMappingModel::getItem(int row)
    {
        return m_data.at(row);
    }

    int DirMappingModel::append()
    {
        int size = rowCount();
        beginInsertRows(QModelIndex(),size ,size);
        DirMappingItem item;
        m_data.push_back(item);
        endInsertRows();

        return size;
    }

    void DirMappingModel::remove(const QModelIndex& index)
    {
        int row = index.row();
        if(row >-1 && row < rowCount()){
            beginRemoveRows(QModelIndex(),row,row);
            m_data.removeAt(row);
            endRemoveRows();
        }
    }

    QString DirMappingModel::value()
    {
        return QString();
    }


    /*********************DirMappingView**************************/

    constexpr const  char DirMappingView::SRC[] ;
    constexpr const  char DirMappingView::DST[] ;

    DirMappingView::DirMappingView(QWidget* parent)
        :QTreeView(parent)
    {
        //setStyleSheet("QTreeView::Item{border-bottom:1px solid rgb(128,128,128);}");
        setItemDelegate(new DirMappingDelegate(this));
        m_widget = new QWidget(this);
        QHBoxLayout* layout = new QHBoxLayout();

        QToolButton* plusBtn = new QToolButton(m_widget);
        plusBtn->setIcon(QIcon(QString::fromUtf8(":/Resource/icons/plus.svg")));
        plusBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        layout->addWidget(plusBtn);
        layout->addStretch();

        QToolButton* minusBtn = new QToolButton(m_widget);
        minusBtn->setIcon(QIcon(QString::fromUtf8(":/Resource/icons/minus.svg")));
        minusBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        layout->addWidget(minusBtn);
        layout->addStretch();
        layout->setContentsMargins(2,5,5,0);

        m_widget->setLayout(layout);
        QRect rect = this->geometry();

        m_widgetHeight = plusBtn->geometry().height() - 5;
        m_widgetWidth = m_widgetHeight * 2;

        QRect rc(rect.x(),rect.height() ,m_widgetWidth , m_widgetHeight);

        m_widget->setGeometry(rc);
        m_widget->raise();

        this->setContentsMargins(0,0,0,m_widgetHeight + 3);

        connect(plusBtn,&QPushButton::clicked,this,&DirMappingView::onAdd);
        connect(minusBtn,&QPushButton::clicked,this,&DirMappingView::onRemove);

    }

    QJsonValue DirMappingView::value(){
        QJsonArray array;
        DirMappingModel* model = (DirMappingModel*)this->model();
        QList<DirMappingItem> items = model->getList();
        int headSize = model->columnCount();
        for(auto item:items){
            if(headSize==1){
                if(item.src.isEmpty()==false){
                    QJsonObject object;
                    object.insert(SRC,item.src);
                    array.push_back(object);
                }
            }else{
                if(item.src.isEmpty()==false || item.dst.isEmpty()==false){
                    QJsonObject object;
                    object.insert(SRC,item.src);
                    object.insert(DST,item.dst);
                    array.push_back(object);
                }
            }

        }
        return QJsonValue(array);
    }

    void DirMappingView::setValue(QJsonValue value)
    {
        QList<DirMappingItem> items;
        DirMappingModel* model = (DirMappingModel*)this->model();
        if(value.isArray()){
            QJsonArray array = value.toArray();
            int size = array.size();
            for(int i=0;i<size;i++){
                DirMappingItem item;
                QJsonValue v = array.at(i);
                QJsonObject object = v.toObject();
                if(object.contains(SRC)){
                    item.src = object.value(SRC).toString();
                }
                if(object.contains(DST)){
                    item.dst = object.value(DST).toString();
                }
                items.push_back(item);
            }
        }
        model->setLists(items);
    }

    void DirMappingView::setHeaderLabels(QStringList headers)
    {
        if(this->model()==nullptr){
            DirMappingModel* model = new DirMappingModel(this);
            model->setHeaderLabels(headers);
            this->setModel(model);
        }
    }

    void DirMappingView::onAdd()
    {
        DirMappingModel* model = (DirMappingModel*)this->model();
        if(model!=nullptr){
            int row = model->append();
            QModelIndex index = model->index(row,DirMappingModel::Src);
            edit(index);

            scrollToBottom();
        }
    }

    void DirMappingView::onRemove()
    {
        DirMappingModel* model = (DirMappingModel*)this->model();
        if(model!=nullptr){
            model->remove(this->currentIndex());
        }
    }

    bool DirMappingView::validate()
    {
        DirMappingModel* model = (DirMappingModel*) this->model();
        QList<DirMappingItem> items = model->getList();
        int headSize = model->columnCount();
        for(auto item:items){
            if(item.src.isEmpty()){
                return false;
            }else if(!item.src.endsWith("/")){
                return false;
            }
            if(headSize>1){

                if(item.dst.isEmpty()){
                    return false;
                }else if(!item.dst.endsWith("/")){
                    return false;
                }
            }

        }
        return true;
    }

    bool DirMappingView::isEmpty()
    {
        return model()->rowCount()==0;
    }

    void DirMappingView::resizeEvent(QResizeEvent *event)
    {
         QRect rect = viewport()->geometry();
         QRect rc(rect.x(),rect.height(),m_widgetWidth,m_widgetHeight);
         m_widget->setGeometry(rc);
         QTreeView::resizeEvent(event);
    }


}
