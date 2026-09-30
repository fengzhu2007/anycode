#include "export.h"
#include "viewer3dwindow.h"
#include "storage/addon_storage.h"

#include <QDebug>

static QString id = "addon::3dviewer";

bool install()
{
    ady::AddonStorage db;
    auto one = db.one(id);
    if(one.id > 0){
        one.status = 1;
        one.export_type = ady::AddonLoader::Menu;
        return db.update(one);
    }else{
        one.name = id;
        one.title = QString{"3D Viewer"};
        one.label = one.title;
        one.file = "3DViewer/3DViewer";
        one.status = 1;
        one.is_system = 0;
        one.export_type = ady::AddonLoader::Menu;
        one.version = "1.0";
        one.author = "Official";
        one.description = QObject::tr("3D file viewer addon providing directory browsing, thumbnail generation and 3D visualization.");
        return db.insert(one) > 0;
    }
}

bool uninstall()
{
    ady::AddonStorage db;
    db.del(id);
    return true;
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

    // 分割线
    ady::MenuData sep;
    sep.menu_kind = ady::AddonLoader::Extend;
    sep.position = 1;
    sep.kind = ady::MenuData::Separator;
    menus.append(sep);

    return menus;
}
