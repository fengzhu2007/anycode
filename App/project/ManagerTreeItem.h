#ifndef MANAGERTREEITEM_H
#define MANAGERTREEITEM_H

#include <QList>
#include <QVariant>
namespace ady {
    class ManagerTreeData
    {
    public:
        enum Role {
            PROJECT=0,
            GROUP=1,
            SITE=2,
        };
        long long id;
        QString name;
        QString path;
        Role role;

        ManagerTreeData();
        ManagerTreeData(Role role,long long id,QString name,QString path);

        ManagerTreeData& operator=(const ManagerTreeData& cls);
    };

    class ManagerTreeItem
    {
    public:
        enum Column {
            Name=0,
            Path,
            Max
        };
        explicit ManagerTreeItem(const ManagerTreeData &data, ManagerTreeItem *parentItem = 0);
        ~ManagerTreeItem();

        void updateData(ManagerTreeData &data);

        void appendChild(ManagerTreeItem *child);

        ManagerTreeItem *child(int row);
        void remove(int row);
        int childCount() const;
        int columnCount() const;
        QVariant data(int column) const;
        int row() const;
        ManagerTreeItem *parentItem();
        void clearAll();
        int childIndex(ManagerTreeData::Role role,long long id);
        bool equal(ManagerTreeData::Role role,long long id);
        inline long long dataId(){return this->m_itemData.id;};
        inline ManagerTreeData::Role dataRole(){return this->m_itemData.role;}

    private:
        QList<ManagerTreeItem*> m_childItems;
        ManagerTreeData m_itemData;
        ManagerTreeItem *m_parentItem;
    };

}


#endif // MANAGERTREEITEM_H
