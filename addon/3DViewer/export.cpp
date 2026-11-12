#include "export.h"
#include "viewer3dwindow.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QObject>

static QString id = "addon::3dviewer";

const char* addonInfo()
{
    static QByteArray json;
    if(json.isEmpty()){
        QJsonObject object;
        object.insert("name",id);
        object.insert("title",QStringLiteral("3D Viewer"));
        object.insert("label",QStringLiteral("3D Viewer"));
        object.insert("file",QStringLiteral("3DViewer/3DViewer"));
        object.insert("export_type",QStringLiteral("Menu"));
        object.insert("version",QStringLiteral("1.0"));
        object.insert("author",QStringLiteral("Official"));
        object.insert("description",QObject::tr("3D model viewer addon with directory browsing, cached thumbnails and Qt Quick 3D preview."));
        json = QJsonDocument(object).toJson(QJsonDocument::Compact);
    }
    return json.constData();
}

GetMenusFun getGetMenus()
{
    return &getMenus;
}

QList<ady::MenuData> getMenus(QWidget* parent)
{
    QList<ady::MenuData> menus;

    ady::MenuData data;
    data.menu_kind = ady::AddonLoader::Extend;
    data.position = 0;
    data.kind = ady::MenuData::Action;
    data.ptr.action = new QAction(QString::fromUtf8("3D Viewer"), parent);
    QObject::connect(data.ptr.action, &QAction::triggered, [parent](){
        Viewer3DWindow* w = new Viewer3DWindow(parent);
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->show();
    });
    menus.append(data);

    //
    ady::MenuData sep;
    sep.menu_kind = ady::AddonLoader::Extend;
    sep.position = 1;
    sep.kind = ady::MenuData::Separator;
    menus.append(sep);

    return menus;
}
