#include "diff_editor_widget.h"
#include "core/coreconstants.h"
#include <texteditorconstants.h>
#include <textdocument.h>
#include <fontsettings.h>
#include <QPainter>
#include <QPaintEvent>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QPalette>
#include <QFile>
#include <QStringList>

namespace ady {

class DiffEditorWidgetPrivate {
public:
    cvs::DiffContent diffContent;
    QString filePath;
    int additions = 0;
    int deletions = 0;
};

DiffEditorWidget::DiffEditorWidget(QWidget *parent)
    : CodeEditorView(parent)
{
    d = new DiffEditorWidgetPrivate;

    setupFallBackEditor(Core::Constants::K_DEFAULT_TEXT_EDITOR_ID);
    setReadOnly(true);

    // We draw our own dual-column line numbers, so disable the base class
    // single-column line numbers to avoid overlap and wasted gutter width.
    setLineNumbersVisible(false);
}

DiffEditorWidget::~DiffEditorWidget()
{
    delete d;
}

void DiffEditorWidget::setDiffContent(const cvs::DiffContent &content, const QString &filePath)
{
    d->diffContent = content;
    d->filePath = filePath;
    d->additions = content.additions();
    d->deletions = content.deletions();

    buildDiffDocument();
    applyLineHighlights();
    updateScrollBarMarkers();
}

cvs::DiffContent DiffEditorWidget::diffContent() const
{
    return d->diffContent;
}

QString DiffEditorWidget::filePath() const
{
    return d->filePath;
}

void DiffEditorWidget::clearDiff()
{
    d->diffContent = cvs::DiffContent();
    d->additions = 0;
    d->deletions = 0;

    clearDiffHighlights();

    QTextCursor cursor(document());
    cursor.select(QTextCursor::Document);
    cursor.removeSelectedText();
}

int DiffEditorWidget::additionCount() const
{
    return d->additions;
}

int DiffEditorWidget::deletionCount() const
{
    return d->deletions;
}

void DiffEditorWidget::setShowOldLineNumbers(bool show)
{
    m_showOldLineNumbers = show;
    // Recalculate gutter geometry (extraAreaWidth sets viewport margins)
    extraAreaWidth();
    extraArea()->updateGeometry();
    extraArea()->update();
    viewport()->update();
}

bool DiffEditorWidget::showOldLineNumbers() const
{
    return m_showOldLineNumbers;
}

//----------------------------------------------------------------------------
// buildDiffDocument
//
// Two strategies depending on whether the full file is available on disk:
//
//  1. Full-file view (preferred): reads the complete new file from disk,
//     displays every line, and interleaves deleted lines at their original
//     positions.  Added lines are highlighted green, deleted lines red.
//     This gives the user the entire code with inline change markers.
//
//  2. Diff-only fallback: if the file cannot be read (e.g. it was deleted or
//     is on a remote server), only the diff hunks are shown.
//
// In both cases the libgit2 prefix character (+/-/space) is stripped from
// diff line content so the gutter marker is the sole visual cue.
//----------------------------------------------------------------------------
void DiffEditorWidget::buildDiffDocument()
{
    m_lineInfo.clear();

    QString text;
    int docLine = 0;

    // Attempt to read the full file from disk -------------------------------
    QString fileContent;
    bool hasFullFile = false;

    if (!d->filePath.isEmpty()) {
        QFile file(d->filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            fileContent = QString::fromUtf8(file.readAll());
            hasFullFile = true;
            file.close();
        }
    }

    if (hasFullFile) {
        // --- Strategy 1: full-file view --------------------------------
        QStringList fileLines = fileContent.split('\n');
        // Remove trailing empty element produced by a final newline
        if (!fileLines.isEmpty() && fileLines.last().isEmpty())
            fileLines.removeLast();

        int fileIdx = 0;         // 0-based index into fileLines
        int oldNewOffset = 0;    // oldLineNo = newLineNo + offset (for unchanged lines)

        const auto hunks = d->diffContent.hunks();
        for (const auto &hunk : hunks) {
            int hunkNewStart = hunk.newStart();

            // Emit unchanged file lines before the hunk
            while (fileIdx < hunkNewStart - 1 && fileIdx < fileLines.size()) {
                LineInfo info;
                info.type = cvs::DiffLine::Context;
                info.newLineNo = fileIdx + 1;
                info.oldLineNo = fileIdx + 1 + oldNewOffset;
                m_lineInfo[docLine] = info;
                text += fileLines[fileIdx] + '\n';
                ++docLine;
                ++fileIdx;
            }

            // Process hunk lines
            const auto lines = hunk.lines();
            for (const auto &line : lines) {
                if (line.type() == cvs::DiffLine::Context) {
                    // Context line — take content from the file on disk
                    if (fileIdx < fileLines.size()) {
                        LineInfo info;
                        info.type = cvs::DiffLine::Context;
                        info.oldLineNo = line.oldLineNo();
                        info.newLineNo = line.newLineNo();
                        m_lineInfo[docLine] = info;
                        text += fileLines[fileIdx] + '\n';
                        ++docLine;
                        ++fileIdx;
                        // Track offset for subsequent unchanged lines
                        if (info.newLineNo > 0 && info.oldLineNo > 0)
                            oldNewOffset = info.oldLineNo - info.newLineNo;
                    }
                } else if (line.type() == cvs::DiffLine::Addition) {
                    // Added line — take content from the file on disk
                    if (fileIdx < fileLines.size()) {
                        LineInfo info;
                        info.type = cvs::DiffLine::Addition;
                        info.oldLineNo = -1;
                        info.newLineNo = line.newLineNo();
                        m_lineInfo[docLine] = info;
                        text += fileLines[fileIdx] + '\n';
                        ++docLine;
                        ++fileIdx;
                    }
                } else if (line.type() == cvs::DiffLine::Deletion) {
                    // Deleted line — not in the new file; take from diff
                    QString content = line.content();
                    if (!content.isEmpty()) {
                        QChar first = content.at(0);
                        if (first == '+' || first == '-' || first == ' ')
                            content = content.mid(1);
                    }
                    LineInfo info;
                    info.type = cvs::DiffLine::Deletion;
                    info.oldLineNo = line.oldLineNo();
                    info.newLineNo = -1;
                    m_lineInfo[docLine] = info;
                    text += content + '\n';
                    ++docLine;
                }
            }
        }

        // Emit remaining unchanged file lines after the last hunk
        while (fileIdx < fileLines.size()) {
            LineInfo info;
            info.type = cvs::DiffLine::Context;
            info.newLineNo = fileIdx + 1;
            info.oldLineNo = fileIdx + 1 + oldNewOffset;
            m_lineInfo[docLine] = info;
            text += fileLines[fileIdx] + '\n';
            ++docLine;
            ++fileIdx;
        }
    } else {
        // --- Strategy 2: diff-only fallback -----------------------------
        const auto hunks = d->diffContent.hunks();
        for (const auto &hunk : hunks) {
            // Hunk header
            QString header = hunk.header().trimmed();
            if (header.isEmpty()) {
                header = QString("@@ -%1,%2 +%3,%4 @@")
                             .arg(hunk.oldStart()).arg(hunk.oldCount())
                             .arg(hunk.newStart()).arg(hunk.newCount());
            }
            m_lineInfo[docLine] = LineInfo{};
            m_lineInfo[docLine].isHeader = true;
            text += header + '\n';
            ++docLine;

            // Diff lines
            const auto lines = hunk.lines();
            for (const auto &line : lines) {
                LineInfo info;
                info.type = line.type();
                info.oldLineNo = line.oldLineNo();
                info.newLineNo = line.newLineNo();
                m_lineInfo[docLine] = info;

                QString content = line.content();
                if (!content.isEmpty()) {
                    QChar first = content.at(0);
                    if (first == '+' || first == '-' || first == ' ')
                        content = content.mid(1);
                }
                text += content + '\n';
                ++docLine;
            }
        }
    }

    // Insert text in bulk (block signals to avoid expensive layout passes)
    setUpdatesEnabled(false);
    document()->blockSignals(true);

    QTextCursor cursor(document());
    cursor.select(QTextCursor::Document);
    cursor.insertText(text);

    document()->blockSignals(false);
    setUpdatesEnabled(true);

    // Reset cursor and refresh
    cursor.movePosition(QTextCursor::Start);
    setTextCursor(cursor);
    document()->markContentsDirty(0, document()->characterCount());
    viewport()->update();
}

//----------------------------------------------------------------------------
// extraAreaWidth
//
// Gutter layout (left → right):
//
//   When old line numbers are enabled:
//     [ pad | old-line-col | pad | sep | pad | new-line-col | pad | marker | pad ]
//   When disabled:
//     [ pad | new-line-col | pad | marker | pad ]
//
// The base class width is added first (it handles marks / folding /
// padding and also calls slotUpdateExtraAreaWidth to keep the viewport
// margins in sync).  We then override the viewport margins to include our
// extra columns so text never underlaps the gutter.
//----------------------------------------------------------------------------
int DiffEditorWidget::extraAreaWidth(int *markWidthPtr) const
{
    // Let the base class handle its diff marker width + viewport margins.
    int baseWidth = CodeEditorView::extraAreaWidth(markWidthPtr);

    if (!m_showOldLineNumbers)
        return baseWidth;

    const QFontMetrics fm(font());
    const int digitW = fm.horizontalAdvance('9');

    int maxOld = 1;
    for (const auto &info : m_lineInfo) {
        if (info.oldLineNo > maxOld) maxOld = info.oldLineNo;
    }
    const int oldDigits = QString::number(maxOld).length();
    const int oldColW = digitW * oldDigits;
    const int pad = 4;
    const int sep = 1;

    int extra = pad + oldColW + pad + sep + pad;
    int total = baseWidth + extra;

    const_cast<DiffEditorWidget*>(this)->setViewportMargins(
        isLeftToRight() ? total : 0, 0,
        isLeftToRight() ? 0 : total, 0);

    return total;
}

//----------------------------------------------------------------------------
// extraAreaPaintEvent
//
// When old line numbers are disabled, delegates to the base class which
// draws standard line numbers + diff markers.
//
// When enabled, fully custom painting: [old-line | sep | new-line | marker]
//----------------------------------------------------------------------------
void DiffEditorWidget::extraAreaPaintEvent(QPaintEvent *e)
{
    if (!m_showOldLineNumbers) {
        // No old line numbers — delegate to base class (standard gutter + diff markers)
        CodeEditorView::extraAreaPaintEvent(e);
        return;
    }

    // Dual-column gutter: [old-line | sep | new-line | marker]
    QPainter painter(extraArea());
    painter.fillRect(e->rect(), extraArea()->palette().color(QPalette::Window));

    const QFontMetrics fm(font());
    const int digitW = fm.horizontalAdvance('9');

    int maxOld = 1;
    for (const auto &info : m_lineInfo) {
        if (info.oldLineNo > maxOld) maxOld = info.oldLineNo;
    }
    const int oldDigits = QString::number(maxOld).length();
    const int oldColW = digitW * oldDigits;
    const int pad = 4;
    const int sep = 1;

    int x = pad;
    const int oldX = x;
    x += oldColW + pad;
    const int sepX = x;
    x += sep + pad;

    // Draw the vertical separator between old and new columns
    painter.setPen(QColor(220, 220, 220));
    painter.drawLine(sepX, e->rect().top(), sepX, e->rect().bottom());

    // New line number column and diff marker column (after separator)
    int maxNew = 1;
    for (const auto &info : m_lineInfo) {
        if (info.newLineNo > maxNew) maxNew = info.newLineNo;
    }
    const int newDigits = QString::number(maxNew).length();
    const int newColW = digitW * newDigits;
    const int markerW = digitW + 4;

    const int newX    = x;
    const int markerX = newX + newColW + pad;

    QTextBlock block = firstVisibleBlock();
    QPointF offset = contentOffset();

    while (block.isValid()) {
        QRectF blockRect = blockBoundingRect(block).translated(offset);

        if (blockRect.bottom() >= e->rect().top()
            && blockRect.top() <= e->rect().bottom())
        {
            int blockNo = block.blockNumber();
            auto it = m_lineInfo.find(blockNo);

            if (it != m_lineInfo.end()) {
                const LineInfo &info = it.value();
                const int top = int(blockRect.top());
                const int h   = int(blockRect.height());

                // Old line number
                if (info.oldLineNo > 0) {
                    painter.setPen(QColor(150, 150, 150));
                    painter.drawText(QRect(oldX, top, oldColW, h),
                                     Qt::AlignRight | Qt::AlignVCenter,
                                     QString::number(info.oldLineNo));
                }

                // New line number
                if (info.newLineNo > 0) {
                    painter.setPen(QColor(120, 120, 120));
                    painter.drawText(QRect(newX, top, newColW, h),
                                     Qt::AlignRight | Qt::AlignVCenter,
                                     QString::number(info.newLineNo));
                }

                // Diff marker (+ / -)
                if (!info.isHeader) {
                    switch (info.type) {
                    case cvs::DiffLine::Addition:
                        painter.setPen(QColor(0, 160, 0));
                        painter.drawText(QRect(markerX, top, markerW, h),
                                         Qt::AlignCenter, "+");
                        break;
                    case cvs::DiffLine::Deletion:
                        painter.setPen(QColor(220, 20, 20));
                        painter.drawText(QRect(markerX, top, markerW, h),
                                         Qt::AlignCenter, "-");
                        break;
                    case cvs::DiffLine::Modification:
                        painter.setPen(QColor(200, 140, 0));
                        painter.drawText(QRect(markerX, top, markerW, h),
                                         Qt::AlignCenter, "~");
                        break;
                    case cvs::DiffLine::Context:
                    default:
                        break;
                    }
                }
            }
        }

        offset.ry() += blockRect.height();
        if (offset.y() > height())
            break;

        block = block.next();
    }
}

//----------------------------------------------------------------------------
// paintDiffOverlay
//
// Called by the base class paint pipeline to overlay content on the text
// area.  Reserved for future word-level diff highlighting.
//----------------------------------------------------------------------------
void DiffEditorWidget::paintDiffOverlay(QPainter *painter, const QRect &clip)
{
    Q_UNUSED(painter);
    Q_UNUSED(clip);
}

} // namespace ady
