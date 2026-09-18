#ifndef MDSTYLER_H
#define MDSTYLER_H

#include <QObject>
#include <QColor>
#include <QTimer>
#include <QPointer>
#include <QVector>
#include <QQuickTextDocument>

class QTextTable;
class QTextTableCell;

class MDStyler : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QColor borderColor
                   READ borderColor WRITE setBorderColor NOTIFY styleChanged)
    Q_PROPERTY(QColor headerBg
                   READ headerBg WRITE setHeaderBg NOTIFY styleChanged)
    Q_PROPERTY(QColor rowEvenBg
                   READ rowEvenBg WRITE setRowEvenBg NOTIFY styleChanged)
    Q_PROPERTY(QColor rowOddBg
                   READ rowOddBg WRITE setRowOddBg NOTIFY styleChanged)
    Q_PROPERTY(QColor headerTextColor
                   READ headerTextColor WRITE setHeaderTextColor NOTIFY styleChanged)

    Q_PROPERTY(int cellPadding
                   READ cellPadding WRITE setCellPadding NOTIFY styleChanged)
    Q_PROPERTY(qreal borderWidth
                   READ borderWidth WRITE setBorderWidth NOTIFY styleChanged)

    Q_PROPERTY(int lineHeight
                   READ lineHeight WRITE setLineHeight NOTIFY styleChanged)

public:
    explicit MDStyler(QObject *parent = nullptr);

    Q_INVOKABLE void attach(QQuickTextDocument *doc);
    Q_INVOKABLE void detach(QQuickTextDocument *doc);
    Q_INVOKABLE void scheduleRestyle();
    Q_INVOKABLE void restyle();

    QColor borderColor() const     { return m_borderColor; }
    void setBorderColor(const QColor &c);

    QColor headerBg() const        { return m_headerBg; }
    void setHeaderBg(const QColor &c);

    QColor rowEvenBg() const       { return m_rowEvenBg; }
    void setRowEvenBg(const QColor &c);

    QColor rowOddBg() const        { return m_rowOddBg; }
    void setRowOddBg(const QColor &c);

    QColor headerTextColor() const { return m_headerTextColor; }
    void setHeaderTextColor(const QColor &c);

    int cellPadding() const        { return m_cellPadding; }
    void setCellPadding(int p);

    qreal borderWidth() const      { return m_borderWidth; }
    void setBorderWidth(qreal w);
    int lineHeight() const        { return m_lineHeight; }
    void setLineHeight(int h);




signals:
    void styleChanged();

private:
    void apply();
    void applyToDocument(QQuickTextDocument *doc);
    void styleTable(QTextTable *table);
    void applyLineHeight(QTextDocument *doc);
    void styleCell(QTextTableCell &cell, int row, int col);

    QVector<QPointer<QQuickTextDocument>> m_targets;
    QTimer  *m_timer  = nullptr;

    QColor m_borderColor{"#d0d7de"};
    QColor m_headerBg{"#f6f8fa"};
    QColor m_rowEvenBg{"#ffffff"};
    QColor m_rowOddBg{"#fafbfc"};
    QColor m_headerTextColor{"#24292f"};

    int    m_cellPadding = 10;
    qreal  m_borderWidth = 1.0;
    int    m_lineHeight = 100;

    bool   m_applying = false;
};

#endif // MDSTYLER_H
