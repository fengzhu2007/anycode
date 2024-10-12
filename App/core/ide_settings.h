#ifndef IDESETTINGS_H
#define IDESETTINGS_H
#include <QString>
#include <QJsonArray>
#include <QJsonObject>

namespace ady{
class IDEWindow;
class IDESettingsPrivate;
class IDESettings
{
public:
    static IDESettings* getInstance(IDEWindow* window=nullptr);
    static void destory();

    ~IDESettings();

    bool readFromFile(const QString& filename={});
    bool saveToFile(const QString& filename={});

    void setDockpanes(const QJsonObject& dockpanes);
    void setProjects(const QJsonArray& projects);

    QJsonObject dockpanes();
    QJsonArray projects();

    bool isMaximized();
    int width();
    int height();


private:
    IDESettings(IDEWindow* window=nullptr);

private:
    static IDESettings* instance;
    IDESettingsPrivate* d;
    int m_version;

};
}
#endif // IDESETTINGS_H
