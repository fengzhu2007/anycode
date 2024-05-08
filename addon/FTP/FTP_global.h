#ifndef FTP_GLOBAL_H
#define FTP_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(FTP_LIBRARY)
#  define FTP_EXPORT Q_DECL_EXPORT
#else
#  define FTP_EXPORT Q_DECL_IMPORT
#endif

#endif // FTP_GLOBAL_H
