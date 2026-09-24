#ifndef CODEEDITORVIEW_H
#define CODEEDITORVIEW_H

//#define Scintilla

#include "global.h"
#ifndef Scintilla
#include "texteditor.h"
#else
#include <Qsci/qsciscintilla.h>
#endif

#include <QContextMenuEvent>
#include <QShowEvent>
#include <QResizeEvent>
#include <QTextCursor>
#include <QHash>

namespace Core { class HighlightScrollBarController; }

namespace ady{
namespace cvs { class DiffContent; }
class CodeEditorViewPrivate;

#ifndef Scintilla

class ANYENGINE_EXPORT CodeEditorView : public TextEditor::TextEditorWidget
{
    Q_OBJECT
public:
    explicit CodeEditorView(QWidget* parent);
    ~CodeEditorView();
    void rename(const QString& name);
    //line 0-base column 0-base
    void addSemanticError(int line,int column,int length,const QString& message);
    void clearSemanticError();

    // Set diff content and apply visual annotations (line highlights,
    // scrollbar markers, gutter markers).  When diff is empty, clears
    // all annotations.
    void setDiffHighlights(const cvs::DiffContent &content);
    void clearDiffHighlights();

protected:
    virtual void contextMenuEvent(QContextMenuEvent *e) override;
    virtual void showEvent(QShowEvent *e) override;
    virtual void resizeEvent(QResizeEvent *e) override;

    void clearSemanticErrorMarks();

    // ---- Diff annotation infrastructure (inactive when m_lineInfo is empty) ----

    // Per-line metadata: maps block number to old/new line numbers and diff type
    struct LineInfo {
        int oldLineNo = -1;
        int newLineNo = -1;
        int type = 0;          // cvs::DiffLine::Type
        bool isHeader = false;
    };

    // Individual rendering steps (called by setDiffHighlights, but also
    // available for manual control)
    void applyLineHighlights();
    void updateScrollBarMarkers();

    // Gutter: reserves width for diff markers when diff data is present
    virtual int extraAreaWidth(int *markWidthPtr = nullptr) const;
    // Gutter: paints line numbers + diff markers (+/-/~)
    virtual void extraAreaPaintEvent(QPaintEvent *e);

    QHash<int, LineInfo> m_lineInfo;
    bool m_showOldLineNumbers = true;
    Core::HighlightScrollBarController *m_diffScrollBarController = nullptr;

private:
    CodeEditorViewPrivate* d;

};

#else

class ANYENGINE_EXPORT CodeEditorView : public QsciScintilla
{
    Q_OBJECT
public:
    explicit CodeEditorView(QWidget* parent);
    ~CodeEditorView();
    void rename(const QString& name);

    void gotoLine(int line);
    QTextCursor textCursor();
    void findText(const QString& text,int flags,bool hightlight);
    void replaceText(const QString&before,const QString& after,int flags,bool hightlight);
    void clearHighlights();




protected:
    virtual void contextMenuEvent(QContextMenuEvent *e) override;
    virtual void showEvent(QShowEvent *e) override;
    virtual void resizeEvent(QResizeEvent * e) override;

private:
    CodeEditorViewPrivate* d;

};

#endif




}
#endif // CODEEDITORVIEW_H
