#ifndef CODEEDITORVIEW_H
#define CODEEDITORVIEW_H

#define Scintilla

#include "global.h"
#ifndef Scintilla
#include "texteditor.h"
#else
#include <Qsci/qsciscintilla.h>
#endif

#include <QContextMenuEvent>
#include <QShowEvent>
#include <QTextCursor>

namespace ady{
class CodeEditorViewPrivate;

#ifndef Scintilla

class ANYENGINE_EXPORT CodeEditorView : public TextEditor::TextEditorWidget
{
    Q_OBJECT
public:
    explicit CodeEditorView(QWidget* parent);
    ~CodeEditorView();
    void rename(const QString& name);
    //void delayGoto(int line,int column=0);

protected:
    virtual void contextMenuEvent(QContextMenuEvent *e) override;
    virtual void showEvent(QShowEvent *e) override;

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

private:
    CodeEditorViewPrivate* d;

};

#endif




}
#endif // CODEEDITORVIEW_H
