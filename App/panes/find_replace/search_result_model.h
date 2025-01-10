#ifndef SEARCHRESULTMODEL_H
#define SEARCHRESULTMODEL_H
#include <QAbstractItemModel>
#include <QItemDelegate>
namespace ady{
class SearchResultItem{
public:
    SearchResultItem(int line,const QString& filePath,const QString& searchItem,int matchStart,int matchLength,const QStringList& regexpCapturedTexts);
    int line;
    QString filePath;
    QString searchItem;
    int matchStart;
    int matchLength;
    QStringList regexpCapturedTexts;
};

class SearchResultModelItemPrivate;
class SearchResultModelItem{
public:
    enum Type{
        Root=0,
        File=1,
        Line
    };
    SearchResultModelItem();
    SearchResultModelItem(const QString& filePath,SearchResultModelItem* parent);
    SearchResultModelItem(const QString& searchItem,int line,int matchStart,int matchLength,SearchResultModelItem* parent);
    ~SearchResultModelItem();

    Type type();
    QString filePath();
    QString searchItem();
    int line();
    int matchStart();
    int matchLength();

    SearchResultModelItem* parent();
    int childrenCount();
    int row();
    SearchResultModelItem* childAt(int i);
    QList<SearchResultModelItem*> takeAll();
    void appendItem(SearchResultModelItem* item);
    void appendItems(QList<SearchResultModelItem*> list);

private:
    SearchResultModelItemPrivate* d;

};

class SearchResultModelPrivate;
class SearchResultModel : public QAbstractItemModel
{
public:
    enum Column {
        Name=0,
        Max
    };
    enum Role{
        ResultBeginLineNumberRole = Qt::UserRole+1,
        ContainingFunctionNameRole,
        ResultIconRole,
        ResultBeginColumnNumberRole,
        SearchTermLengthRole,
        ResultHighlightBackgroundColor,
        FunctionHighlightBackgroundColor,
        ResultHighlightForegroundColor,
        FunctionHighlightForegroundColor
    };
    explicit SearchResultModel(QObject* parent=nullptr);
    ~SearchResultModel();

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void appendItems(QList<SearchResultItem> list);
    void clear();

private:
    SearchResultModelPrivate* d;
};


struct LayoutInfo
{
    QRect checkRect;
    QRect pixmapRect;
    QRect textRect;
    QRect lineNumberRect;
    QIcon icon;
    Qt::CheckState checkState;
    QStyleOptionViewItem option;
};


class SearchResultItemDelegate: public QItemDelegate{
public:
    SearchResultItemDelegate(int tabWidth, QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setTabWidth(int width);
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
private:
    LayoutInfo getLayoutInfo(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    int drawLineNumber(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QModelIndex &index) const;
    void drawText(QPainter *painter, const QStyleOptionViewItem &option,
                  const QRect &rect, const QModelIndex &index) const;

    QString m_tabString;
    QColor m_lineNumberBackground;
    QColor m_selectedBackground;
};






}
#endif // SEARCHRESULTMODEL_H
