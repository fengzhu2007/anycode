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
#include <QString>

namespace Core { class HighlightScrollBarController; }
namespace TextEditor { class SyntaxHighlighter; }

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
    // True while the editor shows the read-only diff view (file content with
    // deleted lines interleaved at their positions).
    bool isDiffViewActive() const;

protected:
    virtual void contextMenuEvent(QContextMenuEvent *e) override;
    virtual void showEvent(QShowEvent *e) override;
    virtual void resizeEvent(QResizeEvent *e) override;
    virtual void keyPressEvent(QKeyEvent *e) override;
    TextEditor::LanguageLoader *createLanguageLoader(const QString &languageName, QTextDocument *doc) override;

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
    void updateDiffGutterWidth();

    // Leaves the diff view (or just drops the annotations when not active):
    // optionally restores the saved file content and resets read-only /
    // line-number state to what they were before the diff view opened.
    void resetDiffViewState(bool restoreDocument);

    // Gutter: reserves width for diff markers when diff data is present
    virtual int extraAreaWidth(int *markWidthPtr = nullptr) const;
    // Gutter: reports the diff gutter width to the base class so the
    // viewport margins stay in sync with the widened extra area
    int additionalExtraAreaWidth() const override;
    // Gutter: paints line numbers + diff markers (+/-/~)
    virtual void extraAreaPaintEvent(QPaintEvent *e);

    // Extra width reserved by the diff gutter (0 when no diff data)
    int diffGutterExtraWidth() const;

    QHash<int, LineInfo> m_lineInfo;
    bool m_showOldLineNumbers = true;
    // Diff view state: while active the document holds the rebuilt diff text
    // (the new file with deleted lines interleaved) and the editor read-only.
    bool m_diffViewActive = false;
    QString m_savedFileText;                // file content to restore on exit
    bool m_savedReadOnly = false;
    bool m_savedLineNumbersVisible = true;
    Core::HighlightScrollBarController *m_diffScrollBarController = nullptr;
    TextEditor::SyntaxHighlighter *m_savedHighlighter = nullptr;

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
