#ifndef VIEWER3D_GLOBAL_H
#define VIEWER3D_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(VIEWER3D_LIBRARY)
#  define VIEWER3D_EXPORT Q_DECL_EXPORT
#else
#  define VIEWER3D_EXPORT Q_DECL_IMPORT
#endif

#endif // VIEWER3D_GLOBAL_H
