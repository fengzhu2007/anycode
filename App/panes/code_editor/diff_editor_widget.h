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
    // Custom gutter: dual old/new line numbers + diff markers
    void extraAreaPaintEvent(QPaintEvent *e) override;
    int extraAreaWidth(int *markWidthPtr = nullptr) const override;

    // Reserved for word-level diff highlighting
    void paintDiffOverlay(QPainter *painter, const QRect &clip) override;

private:
    // Build the document: full-file view when the file is readable,
    // otherwise fall back to diff-hunk-only display.
    void buildDiffDocument();

    DiffEditorWidgetPrivate *d;
};

} // namespace ady

#endif // DIFF_EDITOR_WIDGET_H
