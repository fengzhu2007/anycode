#ifndef EXTENSION_ENGINE_H
#define EXTENSION_ENGINE_H

#include <QJSEngine>
namespace ady{

class ExtensionEngine : public QJSEngine
{
public:
    static ExtensionEngine* getInstance();
    static void init(QObject* parent);
    ~ExtensionEngine();

    //QJSValue run(const QString& path);
    static QJSValue run(const QString& path);


private:
    explicit ExtensionEngine(QObject* parent);

private:
    static ExtensionEngine* instance;
};
}
#endif // EXTENSION_ENGINE_H
