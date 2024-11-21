#ifndef LAYOUTSETTINGS_H
#define LAYOUTSETTINGS_H
#include <QString>
#include <QJsonArray>
#include <QJsonObject>

namespace ady{
class IDEWindow;
class LayoutSettingsPrivate;
class LayoutSettings
{
public:
    static LayoutSettings* getInstance(IDEWindow* window=nullptr);
    static void destory();

    ~LayoutSettings();

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
    LayoutSettings(IDEWindow* window=nullptr);

private:
    static LayoutSettings* instance;
    LayoutSettingsPrivate* d;
    int m_version;

};
}
#endif // LAYOUTSETTINGS_H
