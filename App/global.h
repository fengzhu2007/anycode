#ifndef ANYDEPLOYUI_GLOBAL_H
#define ANYDEPLOYUI_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(APP_LIBRARY)
#  define ANYENGINE_EXPORT Q_DECL_EXPORT
#else
#  define ANYENGINE_EXPORT Q_DECL_IMPORT
#endif

#endif // ANYDEPLOYUI_GLOBAL_H
