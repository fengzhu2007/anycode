#include "resource_manager_opened_model.h"
#include <QTreeView>
#include <QModelIndex>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QKeyEvent>
#include <QApplication>
#include <QPainter>
#include <QDebug>

namespace ady{
ResourceManagerOpenedModel* ResourceManagerOpenedModel::instance = nullptr;
ResourceManagerOpenedModel::ResourceManagerOpenedModel(QObject *parent)
    : QAbstractListModel{parent}
{
        m_provider = ResourceManageIconProvider::getInstance();
}


void ResourceManagerOpenedModel::setDataSource(const QStringList& data){
    beginResetModel();
    m_data.clear();
    for(auto one:data){
        m_data<<QPair<QString,QString>(one.split("/").last(),one);
    }
    endResetModel();
}

void ResourceManagerOpenedModel::appendItem(const QString& path){

    int pos = m_data.size();
    beginInsertRows({},pos,pos);
    m_data << QPair<QString,QString>(path.split("/").last(),path);
    endInsertRows();
}

void ResourceManagerOpenedModel::removeItem(const QString& path){
    int i = -1;
    for(auto one:m_data){
        i++;
        if(one.second==path){
            break;
        }
    }
    if(i>=0){
        beginRemoveRows({},i,i);
        m_data.removeAt(i);
        endRemoveRows();
    }
}

void ResourceManagerOpenedModel::updateItem(const QString& oldPath,const QString& newPath){
    int i = -1;
    auto iter = m_data.begin();
    while(iter!=m_data.end()){
        i++;
        if((*iter).second==oldPath){
            (*iter).first = newPath.split("/").last();
            (*iter).second = newPath;
            emit dataChanged(createIndex(i,0),createIndex(i,0),QVector<int>(Qt::DisplayRole));
            break;
        }
        iter++;
    }
}

QVariant ResourceManagerOpenedModel::data(const QModelIndex &index, int role) const {
    if(role==Qt::DisplayRole){
        int row = index.row();
        auto item = m_data.at(row);
        return item.first;
    }else if(role == Qt::DecorationRole){
        int row = index.row();
        auto item = m_data.at(row);
        auto filename = item.first;
        return m_provider->icon(filename.split(".").last());
    }
    return {};
}

int ResourceManagerOpenedModel::rowCount(const QModelIndex &parent) const {
    return m_data.size();
}


void ResourceManagerOpenedModel::init(QObject *parent){
    if(!instance){
        instance = new ResourceManagerOpenedModel(parent);
    }
}

ResourceManagerOpenedModel* ResourceManagerOpenedModel::getInstance(){
    return instance;
}


ResourceManagerOpenedItemDelegate::ResourceManagerOpenedItemDelegate(QObject* parent)
    :QItemDelegate(parent)
{

}

void ResourceManagerOpenedItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const  {
    QStyleOptionViewItem opt = option;
    //qDebug()<<"row:"<<index.row()<<option.state;
    painter->save();

    QRect checkRect;
    QRect pixmapRect{0,0,16,16};
    QRect textRect;
    doLayout(option, &checkRect, &pixmapRect, &textRect, false);

    //draw background
    if (opt.state & QStyle::State_Selected) {
        if(opt.state & QStyle::State_HasFocus){
            painter->fillRect(QRect(0,opt.rect.y(),opt.rect.width() + opt.rect.x(),opt.rect.height()),QColor("#bb007acc"));
            opt.palette.setColor(QPalette::Text, Qt::white);
        }else{
            painter->fillRect(QRect(0,opt.rect.y(),opt.rect.width() + opt.rect.x(),opt.rect.height()),QColor("#bbcccedb"));
        }
        //opt.state &= ~QStyle::State_HasFocus;
        //opt.state &= ~QStyle::State_Selected;
    }else if (opt.state & QStyle::State_MouseOver) {
        opt.state &= ~QStyle::State_MouseOver;
        painter->fillRect(QRect(0,opt.rect.y(),opt.rect.width() + opt.rect.x(),opt.rect.height()),QColor("#99c9DCF5"));
    }
    //draw icon
    auto icon = index.data(Qt::DecorationRole).value<QIcon>();
    if(!icon.isNull())
        QItemDelegate::drawDecoration(painter,opt,pixmapRect,icon.pixmap(pixmapRect.size()));

    this->drawText(painter,opt,textRect,index);
    painter->restore();
}



QSize ResourceManagerOpenedItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {

    return {0,22};
}

void ResourceManagerOpenedItemDelegate::drawText(QPainter *painter,const QStyleOptionViewItem &option,const QRect &rect,const QModelIndex &index) const{
    //bool isSelected = option.state & QStyle::State_Selected;

    auto model = static_cast<const ResourceManagerOpenedModel*>(index.model());

    auto item = model->itemAt(index.row());
    auto filename = item.first;
    auto path = item.second.left(item.second.length() - filename.length() - 1);

    // clip searchTermLength to end of line

    //const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;

    int filenamePixels = option.fontMetrics.horizontalAdvance(filename);
    int pathPixels = option.fontMetrics.horizontalAdvance(path);

    QRect filenameRect(rect);
    filenameRect.setRight(filenameRect.left() + filenamePixels);

    QRect pathRect(rect);
    pathRect.setLeft(filenameRect.right() + 6);
    pathRect.setRight(pathRect.left() + pathPixels);


    QPalette::ColorGroup cg = option.state & QStyle::State_Enabled? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
        cg = QPalette::Inactive;
    QStyleOptionViewItem baseOption = option;


    baseOption.state &= ~QStyle::State_Selected;

    //painter->fillRect(pathRect.adjusted(textMargin, 0, textMargin - 1, 0),QBrush(Qt::lightGray));

        // Text before the highlighting
    QStyleOptionViewItem noHighlightOpt = baseOption;
    noHighlightOpt.rect = filenameRect;
    noHighlightOpt.textElideMode = Qt::ElideNone;
    //if (isSelected)
    //noHighlightOpt.palette.setColor(QPalette::Text, noHighlightOpt.palette.color(cg, QPalette::HighlightedText));
    QItemDelegate::drawDisplay(painter, noHighlightOpt, filenameRect, filename);

    // Highlight text
    QStyleOptionViewItem pathOpt = noHighlightOpt;
    pathOpt.textElideMode = Qt::ElideRight;
    QFont font = option.font;
    font.setPointSize(8);
    pathOpt.font = font;

    if((option.state&QStyle::State_Selected)==QStyle::State_Selected){
        if(option.state & QStyle::State_HasFocus){
            pathOpt.palette.setColor(QPalette::Text, Qt::white);
        }else{
            pathOpt.palette.setColor(QPalette::Text, Qt::darkGray);
        }
    }else{
        pathOpt.palette.setColor(QPalette::Text, Qt::gray);
    }
    QItemDelegate::drawDisplay(painter, pathOpt, pathRect, path);

}


}
