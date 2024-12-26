#ifndef S3_GLOBAL_H
#define S3_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(S3_LIBRARY)
#  define S3_EXPORT Q_DECL_EXPORT
#else
#  define S3_EXPORT Q_DECL_IMPORT
#endif

#endif // S3_GLOBAL_H
