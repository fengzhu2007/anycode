#ifndef SELECT_TREE_MODEL_H
#define SELECT_TREE_MODEL_H

#include <QAbstractItemModel>
#include <QDebug>
namespace ady{
class SelectTreeModelItem{
public:
    enum Type{
        Root,
        Project,
        Site
    };

    SelectTreeModelItem(){
        m_type = Root;
        m_id = 0ll;
        m_parent = nullptr;
        m_state = Qt::Unchecked;
    };
    SelectTreeModelItem(Type type,long long id,const QString& title,SelectTreeModelItem* parent){
        m_type = type;
        m_id = id;
        m_title = title;
        m_parent = parent;
        m_state = Qt::Unchecked;
    }

    int row(){
        return m_parent->m_children.indexOf(this);
    }

    QList<SelectTreeModelItem*> checkedList(){
        QList<SelectTreeModelItem*> list;
        for(auto one:m_children){
            if(one->state()!=Qt::Unchecked){
                list<<one;
            }
        }
        return list;
    }

    void checkedAll(bool state){
        m_state = state?Qt::Checked:Qt::Unchecked;
        for(auto one:m_children){
            one->checkedAll(state);
        }
    }

    Qt::CheckState state(){
        int size = m_children.size();
        if(size>0){
            int ret = 0;
            for(auto child:m_children){
                if(child->m_state==Qt::Unchecked){
                    ret -= 1;
                }else if(child->m_state==Qt::Checked){
                    ret += 1;
                }
            }
            if(ret>0 && size==ret){
                return Qt::Checked;
            }else if(ret<0 && size==-ret){
                return Qt::Unchecked;
            }else{
                return Qt::PartiallyChecked;
            }
        }else{
            return m_state;
        }
    }

    void dump(const QString& prefix={}){
        qDebug()<<prefix<<m_type<<m_title<<this;
        for(auto one:m_children){
            one->dump(prefix+"--");
        }
    }


    Type m_type;
    long long m_id;
    QString m_title;
    Qt::CheckState m_state;
    SelectTreeModelItem* m_parent;
    QList<SelectTreeModelItem*>m_children;


    ~SelectTreeModelItem(){
        qDeleteAll(m_children);
    }
};

class SelectTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Columns{
        Title,
        Max
    };
    explicit SelectTreeModel(QObject *parent = nullptr);
    ~SelectTreeModel();

    // Header:
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void checkedAll(bool state);

    QString readFile(const QString& path);


public:
    SelectTreeModelItem* m_root;
};

}
#endif // SELECT_TREE_MODEL_H
