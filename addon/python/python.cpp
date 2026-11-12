#include "python.h"
#include "loader.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QObject>

static QString id = "texteditor::python";

// Addon exported functions (resolved by AddonLoader via QLibrary)
const char* addonInfo()
{
    static QByteArray json;
    if(json.isEmpty()){
        QJsonObject object;
        object.insert("name",id);
        object.insert("title",QStringLiteral("Texteditor::Python"));
        object.insert("label",QStringLiteral("Python"));
        object.insert("file",QStringLiteral("python/python"));
        object.insert("export_type",QStringLiteral("CodeAutoComplate"));
        object.insert("version",QStringLiteral("1.0"));
        object.insert("author",QStringLiteral("Official"));
        object.insert("description",QObject::tr("Python language support addon providing syntax highlighting, code indentation and grammar checking for the code editor."));
        json = QJsonDocument(object).toJson(QJsonDocument::Compact);
    }
    return json.constData();
}

TextEditor::LanguageLoader* createLanguageLoader(QString languageName, QTextDocument* doc)
{
    if(languageName=="Python"){
        return new Python::Loader(doc);
    }
    return nullptr;

}
