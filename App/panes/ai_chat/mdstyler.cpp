#include "MDStyler.h"

#include <QtQuick/private/qquicktextedit_p_p.h>

#include <QTextDocument>
#include <QTextTable>
#include <QTextTableCell>
#include <QTextTableFormat>
#include <QTextFrame>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QDebug>

MDStyler::MDStyler(QObject *parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(0);   // 下一轮事件循环执行
    connect(m_timer, &QTimer::timeout, this, &MDStyler::restyle);
}

// ---------- attach / detach ----------

void MDStyler::attach(QQuickTextDocument *doc)
{
    if (!doc) return;

    for (const auto &p : m_targets) {
        if (p.data() == doc) return;
    }

    if (auto *edit = qobject_cast<QQuickTextEdit*>(doc->parent())) {
        connect(edit, &QQuickTextEdit::contentSizeChanged,
                this, &MDStyler::scheduleRestyle);
    }
    m_targets.append(QPointer<QQuickTextDocument>(doc));
    scheduleRestyle();
}

void MDStyler::detach(QQuickTextDocument *doc)
{
    if (!doc) return;

    for (int i = m_targets.size() - 1; i >= 0; --i) {
        if (m_targets[i].data() == doc)
            m_targets.removeAt(i);
    }
}

// ---------- 调度 ----------

void MDStyler::scheduleRestyle()
{
    if (m_paused) {
        m_dirty = true;
        return;
    }
    m_timer->start();
}

void MDStyler::restyle()
{
    //qDebug()<<"restylerestylerestylerestyle";
    apply();
}

// ---------- 批量应用 ----------

void MDStyler::apply()
{
    if (m_applying) return;
    if (m_targets.isEmpty()) return;

    m_applying = true;
    m_targets.erase(
        std::remove_if(m_targets.begin(), m_targets.end(),
                       [](const QPointer<QQuickTextDocument> &p) {
                           return p.isNull();
                       }),
        m_targets.end());

    for (auto &p : m_targets) {
        if (auto *doc = p.data())
            applyToDocument(doc);
    }

    m_applying = false;
}

// ---------- 单文档处理 ----------

void MDStyler::applyToDocument(QQuickTextDocument *qtd)
{
    QTextDocument *doc = qtd->textDocument();
    if (!doc) return;

    applyLineHeight(doc);
    QVector<QTextTable*> tables;
    QTextFrame *root = doc->rootFrame();
    for (QTextFrame::iterator it = root->begin(); !it.atEnd(); ++it) {
        if (QTextTable *t = qobject_cast<QTextTable*>(it.currentFrame()))
            tables.append(t);
    }

    if (tables.isEmpty()) return;

    // 一次 edit block 包住所有 setFormat
    {
        QTextCursor cursor(doc);
        cursor.beginEditBlock();
        for (QTextTable *t : tables)
            styleTable(t);
        cursor.endEditBlock();
    }
    QPointer<QQuickTextEdit> editPtr(qobject_cast<QQuickTextEdit*>(qtd->parent()));
    if (editPtr) {
        if (auto *d = QQuickTextEditPrivate::get(editPtr)) {
            //d->dirty = true;
            for (auto it = d->textNodeMap.begin(); it != d->textNodeMap.end(); ++it)
                it->setDirty();
        }
        //editPtr->update();
    }

}

// ---------- 样式属性 ----------

void MDStyler::setBorderColor(const QColor &c)
{
    if (m_borderColor == c) return;
    m_borderColor = c;
    emit styleChanged();
    scheduleRestyle();
}

void MDStyler::setHeaderBg(const QColor &c)
{
    if (m_headerBg == c) return;
    m_headerBg = c;
    emit styleChanged();
    scheduleRestyle();
}

void MDStyler::setRowEvenBg(const QColor &c)
{
    if (m_rowEvenBg == c) return;
    m_rowEvenBg = c;
    emit styleChanged();
    scheduleRestyle();
}

void MDStyler::setRowOddBg(const QColor &c)
{
    if (m_rowOddBg == c) return;
    m_rowOddBg = c;
    emit styleChanged();
    scheduleRestyle();
}

void MDStyler::setHeaderTextColor(const QColor &c)
{
    if (m_headerTextColor == c) return;
    m_headerTextColor = c;
    emit styleChanged();
    scheduleRestyle();
}

void MDStyler::setCellPadding(int p)
{
    if (m_cellPadding == p) return;
    m_cellPadding = p;
    emit styleChanged();
    scheduleRestyle();
}

void MDStyler::setBorderWidth(qreal w)
{
    if (qFuzzyCompare(m_borderWidth, w)) return;
    m_borderWidth = w;
    emit styleChanged();
    scheduleRestyle();
}


void MDStyler::setLineHeight(int h){
    if (m_lineHeight== h) return;
    m_lineHeight = h;
    emit styleChanged();
    scheduleRestyle();
}

void MDStyler::setPaused(bool p)
{
    if (m_paused == p) return;
    m_paused = p;
    emit pausedChanged();
    if (!m_paused && m_dirty) {
        m_dirty = false;
        scheduleRestyle();
    }
}


void MDStyler::styleTable(QTextTable *table)
{
    const int rows = table->rows();
    const int cols = table->columns();
    if (rows <= 0 || cols <= 0) return;

    QTextTableFormat tf = table->format();
    tf.setBorder(m_borderWidth);
    tf.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    tf.setBorderBrush(m_borderColor);
    tf.setCellPadding(m_cellPadding);
    tf.setCellSpacing(10);
    table->setFormat(tf);

    /*for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            QTextTableCell cell = table->cellAt(r, c);
            styleCell(cell, r, c);
        }
    }*/
}

void MDStyler::applyLineHeight(QTextDocument *doc){

    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        QTextBlockFormat bf = b.blockFormat();
        if (bf.lineHeight() == m_lineHeight &&
            bf.lineHeightType() == QTextBlockFormat::ProportionalHeight)
            continue;
        bf.setLineHeight(m_lineHeight, QTextBlockFormat::ProportionalHeight);
        QTextCursor cur(b);
        cur.setBlockFormat(bf);
    }

}

void MDStyler::styleCell(QTextTableCell &cell, int row, int col)
{
    Q_UNUSED(col)

    const bool isHeader = (row == 0);

    QTextTableCellFormat cf;
    cf.setBackground(isHeader
                         ? m_headerBg
                         : (row % 2 ? m_rowEvenBg : m_rowOddBg));

    cf.setLeftPadding(m_cellPadding);
    cf.setRightPadding(m_cellPadding);
    cf.setTopPadding(m_cellPadding / 2);
    cf.setBottomPadding(m_cellPadding / 2);
    cell.setFormat(cf);

    if (isHeader) {
        QTextCursor cursor = cell.firstCursorPosition();
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

        QTextCharFormat headerFmt;
        headerFmt.setFontWeight(QFont::Bold);
        headerFmt.setForeground(m_headerTextColor);
        cursor.mergeCharFormat(headerFmt);
    }
}
