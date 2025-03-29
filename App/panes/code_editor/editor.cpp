#include "editor.h"
#include "code_lint.h"
#include "code_editor_view.h"
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

void Editor::checkSemantic(){
    auto editor = this->editor();
    auto infolist = CodeLint::checking(editor->textDocument());
    editor->clearSemanticError();//clear before
    if(infolist.length()>0){
        for(auto info:infolist){
            if(info.level!=CodeErrorInfo::None){
                //mark error
                editor->addSemanticError(info.line,info.column,info.length,info.message);
            }else{
                editor->clearSemanticError();
            }
        }
    }
}

}
