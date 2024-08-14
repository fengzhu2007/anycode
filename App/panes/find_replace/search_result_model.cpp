#include "search_result_model.h"
#include <QPainter>
#include <QApplication>
#include <QPalette>
#include <QDebug>
namespace ady{

SearchResultItem::SearchResultItem(int line,const QString& filePath,const QString& searchItem,int matchStart,int matchLength,const QStringList& regexpCapturedTexts)
    :line(line),filePath(filePath),searchItem(searchItem),matchStart(matchStart),matchLength(matchLength),regexpCapturedTexts(regexpCapturedTexts){

}

class SearchResultModelItemPrivate{
public:
    SearchResultModelItem::Type type;
    int line;
    int matchStart;
    int matchLength;
    QString filePath;
    QString searchItem;
    SearchResultModelItem* parent;
    QList<SearchResultModelItem*>children;
};

SearchResultModelItem::SearchResultModelItem(){
    d = new SearchResultModelItemPrivate{SearchResultModelItem::Root};
    d->parent = nullptr;
}

SearchResultModelItem::SearchResultModelItem(const QString& filePath,SearchResultModelItem* parent){
    d = new SearchResultModelItemPrivate{SearchResultModelItem::File};
    d->parent = parent;
    d->filePath = filePath;
}

SearchResultModelItem::SearchResultModelItem(const QString& searchItem,int line,int matchStart,int matchLength,SearchResultModelItem* parent){
    d = new SearchResultModelItemPrivate{SearchResultModelItem::Line,line};
    d->searchItem = searchItem;
    d->parent = parent;
    d->line = line;
    d->matchStart = matchStart;
    d->matchLength = matchLength;
}

SearchResultModelItem::~SearchResultModelItem(){
    delete d;
}

SearchResultModelItem::Type SearchResultModelItem::type(){
    return d->type;
}

QString SearchResultModelItem::filePath(){
    if(d->type==File){
        return d->filePath;
    }else if(d->type==Line){
        return d->parent->filePath();
    }else{
        return {};
    }

}

QString SearchResultModelItem::searchItem(){
    if(d->type==Line){
        return d->searchItem;
    }else{
        return {};
    }
}

int SearchResultModelItem::line(){
    return d->line;
}

int SearchResultModelItem::matchStart(){
    return d->matchStart;
}

int SearchResultModelItem::matchLength(){
    return d->matchLength;
}


SearchResultModelItem* SearchResultModelItem::parent(){
    return d->parent;
}

int SearchResultModelItem::childrenCount(){
    return d->children.size();
}

int SearchResultModelItem::row(){
    int i = -1;
    if(d->parent!=nullptr){
        for(auto one:d->parent->d->children){
            i+=1;
            if(one==this){
                break;
            }
        }
    }
    return i;
}

SearchResultModelItem* SearchResultModelItem::childAt(int i){
    return d->children.at(i);
}

QList<SearchResultModelItem*> SearchResultModelItem::takeAll(){
    auto list = d->children;
    d->children.clear();
    return list;
}

void SearchResultModelItem::appendItem(SearchResultModelItem* item){
    d->children<<item;
}

void SearchResultModelItem::appendItems(QList<SearchResultModelItem*> list){
    for(auto one:list){
        d->children <<one;
    }
}

/******************SearchResultModelItem start *******************/


class SearchResultModelPrivate{
public:
    SearchResultModelItem* root;

};

SearchResultModel::SearchResultModel(QObject* parent)
    :QAbstractItemModel(parent)
{
    d = new SearchResultModelPrivate;
    d->root = new SearchResultModelItem();
}

SearchResultModel::~SearchResultModel(){
    delete d->root;
    delete d;
}

// Header:
QVariant SearchResultModel::headerData(int section, Qt::Orientation orientation, int role ) const {
    if(role==Qt::DisplayRole){
        if(section==Name){
            return tr("Name");
        }
    }
    return {};
}
// Basic functionality:
QModelIndex SearchResultModel::index(int row, int column,const QModelIndex &parent ) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    SearchResultModelItem *parentItem;

    if (!parent.isValid())
        parentItem = d->root;
    else
        parentItem = static_cast<SearchResultModelItem*>(parent.internalPointer());

    SearchResultModelItem *childItem = parentItem->childAt(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex SearchResultModel::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return QModelIndex();

    SearchResultModelItem *childItem = static_cast<SearchResultModelItem*>(index.internalPointer());
    if(childItem==d->root || childItem==nullptr){
        return QModelIndex();
    }

    SearchResultModelItem *parentItem = childItem->parent();
    if (parentItem == d->root || parentItem==nullptr)
        return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}

int SearchResultModel::rowCount(const QModelIndex &parent) const {
    if (!parent.isValid())
        return d->root->childrenCount();

    SearchResultModelItem* parentItem = static_cast<SearchResultModelItem*>(parent.internalPointer());
    if(parentItem!=nullptr){
        return parentItem->childrenCount();
    }else{
        return 0;
    }
}

int SearchResultModel::columnCount(const QModelIndex &parent) const{
    return Max;
}

QVariant SearchResultModel::data(const QModelIndex &index, int role) const{
    if (!index.isValid())
        return QVariant();
    if(role == Qt::DisplayRole){
        SearchResultModelItem* item = static_cast<SearchResultModelItem*>(index.internalPointer());
        int col = index.column();
        SearchResultModelItem::Type type = item->type();
        if(col==Name){
            if(type==SearchResultModelItem::File){
                return item->filePath();
            }else{
                return item->searchItem();
            }
        }
    }else if(role==ResultBeginLineNumberRole){
        SearchResultModelItem* item = static_cast<SearchResultModelItem*>(index.internalPointer());
        SearchResultModelItem::Type type = item->type();
        if(type==SearchResultModelItem::Line){
            return item->line();
        }else{
            return 0;
        }
        /*
                ResultBeginLineNumberRole = Qt::UserRole+1,
                ContainingFunctionNameRole,
                ResultIconRole,
                ResultBeginColumnNumberRole,
                SearchTermLengthRole,
                ResultHighlightBackgroundColor,
                FunctionHighlightBackgroundColor,
                ResultHighlightForegroundColor,
                FunctionHighlightForegroundColor
        */
    }else if(role==ResultBeginColumnNumberRole){
        SearchResultModelItem* item = static_cast<SearchResultModelItem*>(index.internalPointer());
        SearchResultModelItem::Type type = item->type();
        if(type==SearchResultModelItem::Line){
            return item->matchStart();
        }else{
            return 0;
        }
    }else if(role==SearchTermLengthRole){
        SearchResultModelItem* item = static_cast<SearchResultModelItem*>(index.internalPointer());
        SearchResultModelItem::Type type = item->type();
        if(type==SearchResultModelItem::Line){
            return item->matchLength();
        }else{
            return 0;
        }
    }else if(role==ResultHighlightBackgroundColor){
        return QColor(244,167,33);
    }
    return QVariant();
}

void SearchResultModel::appendItems(QList<SearchResultItem> list){
    //auto filelist = d->root->
    QList<SearchResultModelItem*> filelist;
    QString temp;
    SearchResultModelItem* v;
    for(auto one:list){
        if(temp!=one.filePath){
            temp = one.filePath;
            v = new SearchResultModelItem(one.filePath,d->root);
            filelist << v;
        }
        auto vv = new SearchResultModelItem(one.searchItem,one.line,one.matchStart,one.matchLength,v);
        v->appendItem(vv);
    }
    int position = d->root->childrenCount();
    if(filelist.size()>0){
        QModelIndex index;
        beginInsertRows(index,position,position + filelist.size() - 1);
        d->root->appendItems(filelist);
        endInsertRows();
    }
}


void SearchResultModel::clear(){
    QModelIndex index;
    int childrenCount = d->root->childrenCount();
    if(childrenCount>0){
        beginRemoveRows(index,0,childrenCount - 1);
        auto rows = d->root->takeAll();
        endRemoveRows();
        qDeleteAll(rows);
    }
}








/******************SearchResultItemDelegate start *******************************/


SearchResultItemDelegate::SearchResultItemDelegate(int tabWidth, QObject *parent)
    : QItemDelegate(parent)
{
    setTabWidth(tabWidth);
}

const int lineNumberAreaHorizontalPadding = 4;
const int minimumLineNumberDigits = 6;

static std::pair<int, QString> lineNumberInfo(const QStyleOptionViewItem &option,
                                              const QModelIndex &index)
{
    const int lineNumber = index.data(SearchResultModel::ResultBeginLineNumberRole).toInt();
    if (lineNumber < 1)
        return {0, {}};
    const QString lineNumberText = QString::number(lineNumber);
    const int lineNumberDigits = qMax(minimumLineNumberDigits, lineNumberText.count());
    const int fontWidth = option.fontMetrics.horizontalAdvance(QString(lineNumberDigits, QLatin1Char('0')));
    const QStyle *style = option.widget ? option.widget->style() : QApplication::style();
    return {lineNumberAreaHorizontalPadding + fontWidth + lineNumberAreaHorizontalPadding
                + style->pixelMetric(QStyle::PM_FocusFrameHMargin),
            lineNumberText};
}

// Aligns text by appending spaces
static QPair<QString, QString> align(QString text, const QString& containingFunction) {
    constexpr int minimumTextSize = 80;
    constexpr int textSizeIncrement = 20;

    int textSize = ((text.size() / textSizeIncrement) + 1) * textSizeIncrement;
    textSize = std::max(minimumTextSize, textSize);
    text.resize(textSize, ' ');
    return QPair<QString, QString>{std::move(text), containingFunction};
}

static QPair<QString, QString> itemText(const QModelIndex &index)
{
    QString text = index.data(Qt::DisplayRole).toString();
    // show number of subresults in displayString
    QString containingFunction;
    const auto contFnName = index.data(SearchResultModel::ContainingFunctionNameRole).toString();
    if (contFnName.length())
        containingFunction = QLatin1String("[in ") + contFnName + QLatin1String("]");

    if (index.model()->hasChildren(index)) {
        QString textAndCount{text + QLatin1String(" (")
                             + QString::number(index.model()->rowCount(index)) + QLatin1Char(')')};

        return align(std::move(textAndCount), containingFunction);
    }
    return align(std::move(text), containingFunction);
}

LayoutInfo SearchResultItemDelegate::getLayoutInfo(const QStyleOptionViewItem &option,
                                                       const QModelIndex &index) const
{
    static const int iconSize = 16;

    LayoutInfo info;
    info.option = setOptions(index, option);

    // check mark
    const bool checkable = (index.model()->flags(index) & Qt::ItemIsUserCheckable);
    info.checkState = Qt::Unchecked;
    if (checkable) {
        QVariant checkStateData = index.data(Qt::CheckStateRole);
        info.checkState = static_cast<Qt::CheckState>(checkStateData.toInt());
        info.checkRect = doCheck(info.option, info.option.rect, checkStateData);
    }

    // icon
    info.icon = index.data(SearchResultModel::ResultIconRole).value<QIcon>();
    if (!info.icon.isNull()) {
        const QSize size = info.icon.actualSize(QSize(iconSize, iconSize));
        info.pixmapRect = QRect(0, 0, size.width(), size.height());
    }

    // text
    /*info.textRect = info.option.rect.adjusted(0,
                                              0,
                                              info.checkRect.width() + info.pixmapRect.width(),
                                              0);*/

    // do basic layout
    doLayout(info.option, &info.checkRect, &info.pixmapRect, &info.textRect, false);

    // adapt for line numbers
    const int lineNumberWidth = lineNumberInfo(info.option, index).first;
    info.lineNumberRect = info.textRect;
    info.lineNumberRect.setWidth(lineNumberWidth);
    info.textRect.adjust(lineNumberWidth, 0, 0, 0);
    return info;
}

void SearchResultItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    //qDebug()<<"rect:"<<option.rect;
    const LayoutInfo info = getLayoutInfo(option, index);

    painter->setFont(info.option.font);

    QItemDelegate::drawBackground(painter, info.option, index);

    // ---- draw the items
    // icon
    //if (!info.icon.isNull())
    //    info.icon.paint(painter, info.pixmapRect, info.option.decorationAlignment);

    // line numbers
    drawLineNumber(painter, info.option, info.lineNumberRect, index);


    drawText(painter, info.option, info.textRect, index);


    //QItemDelegate::drawFocus(painter, info.option, info.option.rect);

    // check mark
    //if (info.checkRect.isValid())
   //     QItemDelegate::drawCheck(painter, info.option, info.checkRect, info.checkState);

    painter->restore();
}

void SearchResultItemDelegate::setTabWidth(int width)
{
    m_tabString = QString(width, QLatin1Char(' '));
}

QSize SearchResultItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                             const QModelIndex &index) const
{
    const LayoutInfo info = getLayoutInfo(option, index);
    //const int height = index.data(Qt::SizeHintRole).value<QSize>().height();
    const int height = 20;
    // get text width, see QItemDelegatePrivate::displayRect
    auto texts = itemText(index);
    const QString text = texts.first.replace('\t', m_tabString)
                         + texts.second.replace('\t', m_tabString);
    const QRect textMaxRect(0, 0, INT_MAX / 256, height);
    const QRect textLayoutRect = textRectangle(nullptr, textMaxRect, info.option.font, text);
    const QRect textRect(info.textRect.x(), info.textRect.y(), textLayoutRect.width(), height);
    const QRect layoutRect = info.checkRect | info.pixmapRect | info.lineNumberRect | textRect;
    //return QSize(layoutRect.x(), layoutRect.y()) + layoutRect.size();
    return QSize(0, height);
}

// returns the width of the line number area
int SearchResultItemDelegate::drawLineNumber(QPainter *painter, const QStyleOptionViewItem &option,
                                                 const QRect &rect,
                                                 const QModelIndex &index) const
{
    const bool isSelected = option.state & QStyle::State_Selected;
    const std::pair<int, QString> numberInfo = lineNumberInfo(option, index);
    if (numberInfo.first == 0)
        return 0;
    QRect lineNumberAreaRect(rect);
    lineNumberAreaRect.setWidth(numberInfo.first);

    QPalette::ColorGroup cg = QPalette::Normal;
    if (!(option.state & QStyle::State_Active))
        cg = QPalette::Inactive;
    else if (!(option.state & QStyle::State_Enabled))
        cg = QPalette::Disabled;

    if(isSelected){
        painter->fillRect(lineNumberAreaRect.adjusted(-lineNumberAreaRect.left(),0,0,0),QColor(147,195,233));
    }else{
        painter->fillRect(lineNumberAreaRect,QColor(221,221,221));
    }

    QStyleOptionViewItem opt = option;
    opt.state &= ~QStyle::State_Selected;
    opt.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
    opt.palette.setColor(cg, QPalette::Text, Qt::darkGray);

    const QStyle *style = QApplication::style();
    const int textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, nullptr) + 1;

    const QRect rowRect = lineNumberAreaRect.adjusted(-textMargin, 0, textMargin-lineNumberAreaHorizontalPadding, 0);
    QItemDelegate::drawDisplay(painter, opt, rowRect, numberInfo.second);

    return numberInfo.first;
}

void SearchResultItemDelegate::drawText(QPainter *painter,
                                            const QStyleOptionViewItem &option,
                                            const QRect &rect,
                                            const QModelIndex &index) const
{
    bool isSelected = option.state & QStyle::State_Selected;
    const auto texts = itemText(index);
    const QString text = texts.first;

    const int searchTermStart = index.model()->data(index, SearchResultModel::ResultBeginColumnNumberRole).toInt();
    int searchTermLength = index.model()->data(index, SearchResultModel::SearchTermLengthRole).toInt();
    if (searchTermStart < 0 || searchTermStart >= text.length() || searchTermLength < 1) {
        QStyleOptionViewItem baseOption = option;
        baseOption.state &= ~QStyle::State_Selected;
        if(isSelected){
            painter->fillRect(rect.adjusted(-rect.left(), 0, 0, 0),QColor(147,195,233));
            //baseOption.palette.setColor(QPalette::Text, Qt::white);
        }

        QItemDelegate::drawDisplay(painter,
                                   baseOption,
                                   rect,
                                   QString(text).replace(QLatin1Char('\t'), m_tabString));
        return;
    }

    // clip searchTermLength to end of line
    searchTermLength = qMin(searchTermLength, text.length() - searchTermStart);
    const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
    const QString textBefore = text.left(searchTermStart).replace(QLatin1Char('\t'), m_tabString);
    const QString textHighlight = text.mid(searchTermStart, searchTermLength).replace(QLatin1Char('\t'), m_tabString);
    const QString textAfter = text.mid(searchTermStart + searchTermLength).replace(QLatin1Char('\t'), m_tabString);
    int searchTermStartPixels = option.fontMetrics.horizontalAdvance(textBefore);
    int searchTermLengthPixels = option.fontMetrics.horizontalAdvance(textHighlight);
    int textAfterLengthPixels = option.fontMetrics.horizontalAdvance(textAfter);
    //int containingFunctionLengthPixels = option.fontMetrics.horizontalAdvance(texts.second);
    //qDebug()<<"rect:"<<rect;
    // rects
    QRect beforeHighlightRect(rect);
    beforeHighlightRect.setRight(beforeHighlightRect.left() + searchTermStartPixels);

    QRect resultHighlightRect(rect);
    resultHighlightRect.setLeft(beforeHighlightRect.right());
    resultHighlightRect.setRight(resultHighlightRect.left() + searchTermLengthPixels);

    QRect afterHighlightRect(rect);
    afterHighlightRect.setLeft(resultHighlightRect.right());
    //afterHighlightRect.setRight(afterHighlightRect.left() + textAfterLengthPixels);
    //qDebug()<<beforeHighlightRect<<resultHighlightRect<<afterHighlightRect;

    //QRect containingFunctionRect(rect);
    //containingFunctionRect.setLeft(afterHighlightRect.right());
    //containingFunctionRect.setRight(afterHighlightRect.right() + containingFunctionLengthPixels);

    // paint all highlight backgrounds
    // qitemdelegate has problems with painting background when highlighted
    // (highlighted background at wrong position because text is offset with textMargin)
    // so we duplicate a lot here, see qitemdelegate for reference

    QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                  ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
        cg = QPalette::Inactive;
    QStyleOptionViewItem baseOption = option;
    baseOption.state &= ~QStyle::State_Selected;
    if (isSelected) {
        painter->fillRect(beforeHighlightRect.adjusted(0, 0, textMargin, 0),QColor(147,195,233));
        painter->fillRect(afterHighlightRect.adjusted(textMargin, 0, textMargin, 0),QColor(147,195,233));
    }
    const QColor highlightBackground =
        index.model()->data(index, SearchResultModel::ResultHighlightBackgroundColor).value<QColor>();
    painter->fillRect(resultHighlightRect.adjusted(textMargin, 0, textMargin - 1, 0),
                      QBrush(highlightBackground));


    // Text before the highlighting
    QStyleOptionViewItem noHighlightOpt = baseOption;
    noHighlightOpt.rect = beforeHighlightRect;
    noHighlightOpt.textElideMode = Qt::ElideNone;
    //if (isSelected)
        //noHighlightOpt.palette.setColor(QPalette::Text, noHighlightOpt.palette.color(cg, QPalette::HighlightedText));
    QItemDelegate::drawDisplay(painter, noHighlightOpt, beforeHighlightRect, textBefore);

    // Highlight text
    QStyleOptionViewItem highlightOpt = noHighlightOpt;
    const QColor highlightForeground
        = index.model()->data(index, SearchResultModel::ResultHighlightForegroundColor).value<QColor>();
    highlightOpt.palette.setColor(QPalette::Text, highlightForeground);
    QItemDelegate::drawDisplay(painter, highlightOpt, resultHighlightRect, textHighlight);

    // Text after the Highlight
    noHighlightOpt.rect = afterHighlightRect;
    QItemDelegate::drawDisplay(painter, noHighlightOpt, afterHighlightRect, textAfter);

}







}
