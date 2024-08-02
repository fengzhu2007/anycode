#ifndef CODEEDITORVIEW_H
#define CODEEDITORVIEW_H
#include "global.h"
#include "texteditor.h"
namespace ady{

class ANYENGINE_EXPORT CodeEditorView : public TextEditor::TextEditorWidget
{
    Q_OBJECT
public:
    explicit CodeEditorView(QWidget* parent);
    void rename(const QString& name);

protected:
    virtual void contextMenuEvent(QContextMenuEvent *e) override;

};
}
#endif // CODEEDITORVIEW_H
