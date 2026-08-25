#ifndef DIFF_EDITOR_WIDGET_H
#define DIFF_EDITOR_WIDGET_H

#include "global.h"
#include "code_editor_view.h"
#include "cvs/diff_content.h"
#include <QHash>

namespace ady {

class DiffEditorWidgetPrivate;

class ANYENGINE_EXPORT DiffEditorWidget : public CodeEditorView
{
    Q_OBJECT

public:
    explicit DiffEditorWidget(QWidget *parent = nullptr);
    ~DiffEditorWidget();

    // Set diff data and populate the editor.
    // filePath: path to the file on disk — when readable, the full file is
    // displayed with added/deleted lines highlighted in place.
    void setDiffContent(const cvs::DiffContent &content, const QString &filePath = QString());
    cvs::DiffContent diffContent() const;
    QString filePath() const;

    // Clear diff display
    void clearDiff();

    // Statistics
    int additionCount() const;
    int deletionCount() const;

    // Toggle the old line-number column in the gutter
    void setShowOldLineNumbers(bool show);
    bool showOldLineNumbers() const;

protected:
    // Custom gutter: line numbers and diff markers (+/-)
    // Old line column is shown only when m_showOldLineNumbers is true
    void extraAreaPaintEvent(QPaintEvent *e) override;
    int extraAreaWidth(int *markWidthPtr = nullptr) const override;

    // Reserved for word-level diff highlighting
    void paintDiffOverlay(QPainter *painter, const QRect &clip) override;

    // Sync scrollbar overlay geometry after the base class has updated
    // the scrollbar (the overlay's eventFilter runs too early)
    void resizeEvent(QResizeEvent *e) override;
    void showEvent(QShowEvent *e) override;

private:
    // Build the document: full-file view when the file is readable,
    // otherwise fall back to diff-hunk-only display.
    void buildDiffDocument();

    // Apply background colors to added / deleted / header lines
    void applyLineHighlights();

    // Place diff markers (additions / deletions) on the vertical scrollbar
    void updateScrollBarMarkers();

    // Per-line metadata: maps block number to old/new line numbers and diff type
    struct LineInfo {
        int oldLineNo = -1;
        int newLineNo = -1;
        cvs::DiffLine::Type type = cvs::DiffLine::Context;
        bool isHeader = false;
    };
    QHash<int, LineInfo> m_lineInfo;
    bool m_showOldLineNumbers = true;

    DiffEditorWidgetPrivate *d;
};

} // namespace ady

#endif // DIFF_EDITOR_WIDGET_H
