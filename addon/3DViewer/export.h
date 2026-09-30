#ifndef VIEWER3D_EXPORT_H
#define VIEWER3D_EXPORT_H
#include "3dviewer_global.h"
#include "addon_loader.h"

typedef QList<ady::MenuData> (*GetMenusFun)(QWidget*);

#ifdef __cplusplus
extern "C" {
#endif

VIEWER3D_EXPORT bool install();
VIEWER3D_EXPORT bool uninstall();
VIEWER3D_EXPORT GetMenusFun getGetMenus();

#ifdef __cplusplus
}
#endif

QList<ady::MenuData> getMenus(QWidget* parent);

#endif // VIEWER3D_EXPORT_H
