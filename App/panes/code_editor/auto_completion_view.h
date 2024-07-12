#ifndef AUTOCOMPLETIONVIEW_H
#define AUTOCOMPLETIONVIEW_H
#include "global.h"
#include <QListView>
#include <QAbstractItemModel>
namespace ady{

class AutoCompletionItem{
public:
    enum Type{
        Unknow=0,
        Keyword,
        Class,
        Function,
        Variant,
        Constant,
        Field,
    };
    Type type=Unknow;
    QString name;
};

class Language;
class AutoCompletionModelPrivate;
class AutoCompletionModel : public QAbstractItemModel{
public:

    ~AutoCompletionModel();
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;


    void setDataSource(QVector<AutoCompletionItem> list);
    void initLanguage(Language* language);

    static AutoCompletionModel* getInstance();
    static void init();
    static void destory();


private:
    AutoCompletionModel();

private:
    static AutoCompletionModel* instance;
    AutoCompletionModelPrivate* d;

};


class AutoCompletionViewPrivate;
class ANYENGINE_EXPORT AutoCompletionView : public QListView
{
    Q_OBJECT
public:
    explicit AutoCompletionView(QWidget* parent=nullptr);
    ~AutoCompletionView();
    AutoCompletionModel* model();
private:
    AutoCompletionViewPrivate* d;
};
}

#endif // AUTOCOMPLETIONVIEW_H
