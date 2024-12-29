#include "editor.h"
namespace ady{
class EditorPrivate{
public:
    Editor::Type type;
};

Editor::Editor(Type type,QWidget *parent):DockingPane(parent) {
    d = new EditorPrivate;
    d->type = type;
}

Editor::~Editor(){
    delete d;
}

Editor::Type Editor::type(){
    return d->type;
}

CodeEditorView* Editor::editor(){
    return nullptr;
}


}
