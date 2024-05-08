#ifndef DIRMAPPINGVIEW_H
#define DIRMAPPINGVIEW_H
#include "global.h"
#include <QTreeView>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QJsonValue>
#include <QPen>

namespace ady{

    class ANYENGINE_EXPORT DirMappingItem
    {
    public:
        ///DirMappingItem(QString src,QString dst);
        QString src;
        QString dst;
        unsigned flags;
    };


    class ANYENGINE_EXPORT DirMappingDelegate : public QStyledItemDelegate
    {
    public:
        DirMappingDelegate(QObject* parent=nullptr);


        //virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;


        virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

        //virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    private:
        //QWidget* m_widget;
        QPen m_gridPen;

    };




    class ANYENGINE_EXPORT DirMappingModel : public QAbstractTableModel
    {
    public:
        enum Column {
            Src=0,
            Dst,
            Max
        };

        DirMappingModel(QObject* parent=nullptr);

        virtual QVariant data(const QModelIndex &index, int role) const override;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
        virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        void setLists(QList<DirMappingItem> data);
        inline QList<DirMappingItem> getList(){return m_data;}
        QString value();
        DirMappingItem getItem(int row);

        inline void setHeaderLabels(QStringList headers) {m_header = headers;}

        int append();
        void remove(const QModelIndex& index);


    private:
        QList<DirMappingItem> m_data;
        QStringList m_header;


    };



    class ANYENGINE_EXPORT DirMappingView : public QTreeView
    {
        Q_OBJECT
    public:
        constexpr const static char SRC[] = "src";
        constexpr const static char DST[] = "dst";

        DirMappingView(QWidget* parent=nullptr);
        QJsonValue value();
        void setValue(QJsonValue value);
        void setHeaderLabels(QStringList headers);
        bool validate();
        bool isEmpty();


        virtual void resizeEvent(QResizeEvent *event) override;

    public slots:
        void onAdd();
        void onRemove();




    private:
        QWidget* m_widget;
        int m_widgetWidth = 0;
        int m_widgetHeight = 0;





    };

}
#endif // DIRMAPPINGVIEW_H
