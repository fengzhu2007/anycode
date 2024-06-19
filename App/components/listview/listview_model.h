#ifndef LISTVIEWMODEL_H
#define LISTVIEWMODEL_H
#include "global.h"
#include <QObject>
#include <QFrame>

namespace ady{
class ListViewItemPrivate;
class ListViewItem : public QFrame{
    Q_OBJECT
public:
    enum State{
        Normal=0,
        Selected
    };
    explicit ListViewItem(QWidget* parent);
    virtual ~ListViewItem();
    void setState(State state);
protected:
    virtual void paintEvent(QPaintEvent *e) override;

private:
    ListViewItemPrivate* d;
};

class ListView;
class ListViewModelPrivate;
class ANYENGINE_EXPORT ListViewModel : public QObject
{
    Q_OBJECT
public:
    explicit ListViewModel(ListView *parent = nullptr);
    virtual int count()=0;
    virtual ListViewItem* item(int i);
    ListViewItem* at(int i);
    void addWidget(ListViewItem* widget);
    ListViewItem* takeAt(int i);
    void dataChanged();
private:
    ListViewModelPrivate* d;



signals:

};
}

#endif // LISTVIEWMODEL_H
