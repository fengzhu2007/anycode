#ifndef LISTVIEW_H
#define LISTVIEW_H
#include "global.h"
#include <QScrollArea>
#include <QLayout>
namespace ady{

class ListViewContentLayoutPrivate;
class ANYENGINE_EXPORT ListViewContentLayout : public QLayout{
    Q_OBJECT
public:
    explicit ListViewContentLayout(QWidget* parent, int margin = 0, int spacing = -1);
    virtual void addItem(QLayoutItem *item) override;
    virtual Qt::Orientations expandingDirections() const override;
    virtual bool hasHeightForWidth() const override;
    virtual int count() const override;
    virtual QLayoutItem *itemAt(int index) const override;
    virtual QSize minimumSize() const override;
    virtual void setGeometry(const QRect &rect) override;
    virtual QSize sizeHint() const override;
    virtual QLayoutItem *takeAt(int index) override;
    void addWidget(QWidget *w);
private:
    ListViewContentLayoutPrivate* d;
};


class ListViewPrivate;
class ListViewModel;
class ANYENGINE_EXPORT ListView : public QScrollArea
{
    Q_OBJECT
public:
    explicit ListView(QWidget* parent);
    ~ListView();
    void clearSelected();
    void setModel(ListViewModel* model);
    ListViewModel* model();
    void render();
    void renderItem(int i);
    void removeItem(int i);
    void setSelection(const QList<int>& indexes);
    QList<int> selection();
    int findItem(const QPoint& pos);
protected:
    virtual void mousePressEvent(QMouseEvent *e) override;
    virtual void mouseMoveEvent(QMouseEvent *e) override;
    virtual void mouseReleaseEvent(QMouseEvent *e) override;
    virtual void resizeEvent(QResizeEvent *event) override;


signals:
    void itemClicked(int i);

private:
    void resizeEmptyWidget();

private:
    ListViewPrivate* d;
};
}
#endif // LISTVIEW_H
