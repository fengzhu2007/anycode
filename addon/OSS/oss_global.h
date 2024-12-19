#ifndef OSS_GLOBAL_H
#define OSS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(OSS_LIBRARY)
#  define OSS_EXPORT Q_DECL_EXPORT
#else
#  define OSS_EXPORT Q_DECL_IMPORT
#endif

#endif // OSS_GLOBAL_H
