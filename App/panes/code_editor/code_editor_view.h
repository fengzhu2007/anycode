#ifndef CODEEDITORVIEW_H
#define CODEEDITORVIEW_H

#include "global.h"
#ifndef Scintilla
#include "texteditor.h"
#else
#include <ScintillaEdit.h>
#endif

#include <QContextMenuEvent>
#include <QShowEvent>

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

class ANYENGINE_EXPORT CodeEditorView : public ScintillaEdit
{
    Q_OBJECT
public:
    explicit CodeEditorView(QWidget* parent);
    ~CodeEditorView();
    void rename(const QString& name);


protected:
    virtual void contextMenuEvent(QContextMenuEvent *e) override;
    virtual void showEvent(QShowEvent *e) override;

private:
    CodeEditorViewPrivate* d;

};

#endif




}
#endif // CODEEDITORVIEW_H
