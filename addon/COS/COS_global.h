#ifndef COS_GLOBAL_H
#define COS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(COS_LIBRARY)
#  define COS_EXPORT Q_DECL_EXPORT
#else
#  define COS_EXPORT Q_DECL_IMPORT
#endif

#endif // COS_GLOBAL_H
