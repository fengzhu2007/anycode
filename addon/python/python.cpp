#include "python.h"
#include "loader.h"
#include <storage/addon_storage.h>
#include <addon_loader.h>

static QString id = "texteditor::python";
// Addon exported functions (resolved by AddonLoader via QLibrary)
bool install()
{
    // TODO: Python addon initialization on install
    ady::AddonStorage db;
    auto one = db.one(id);
    if(one.id>0){
        one.status = 1;
        one.export_type = ady::AddonLoader::CodeAutoComplate;
        return db.update(one);
    }else{
        one.name = id;
        one.title = QString{"Texteditor::Python"};
        one.label = one.title;
        one.file = "python/python";
        one.status = 1;
        one.is_system = 0;
        one.export_type = ady::AddonLoader::CodeAutoComplate;
        one.version = "1.0";
        one.author = "Official";
        one.description = QObject::tr("Python language support addon providing syntax highlighting, code indentation and grammar checking for the code editor.");
        return db.insert(one)>0;
    }
}

bool uninstall()
{
    // TODO: Python addon cleanup on uninstall
    ady::AddonStorage db;
    db.del(id);
    return true;
}

TextEditor::LanguageLoader* createLanguageLoader(QString languageName, QTextDocument* doc)
{
    if(languageName=="Python"){
        return new Python::Loader(doc);
    }
    return nullptr;

}
