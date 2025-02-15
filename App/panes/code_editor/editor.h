#ifndef EDITOR_H
#define EDITOR_H
#include <QString>
#include <docking_pane.h>
namespace ady{

class EditorPrivate;
class CodeEditorView;
class Editor :  public DockingPane
{
public:
    enum Type{
        CodeEditor,
        ImageEditor,
        SQLManager,


        Custome
    };
    Editor(Type type,QWidget *parent = nullptr);
    virtual ~Editor() override;
    Type type();
    virtual QString path()=0;
    virtual void rename(const QString& nname)=0;
    virtual void autoSave()=0;
    virtual bool isModification()=0;
    virtual bool readFile(const QString& path)=0;
    virtual int fileState()=0;
    virtual void setFileState(int state)=0;
    virtual void invokeFileState()=0;
    virtual void reload()=0;
    virtual CodeEditorView* editor();

private:
    EditorPrivate* d;

};
}
#endif // EDITOR_H
