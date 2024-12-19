#ifndef SFTP_GLOBAL_H
#define SFTP_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(SFTP_LIBRARY)
#  define SFTP_EXPORT Q_DECL_EXPORT
#else
#  define SFTP_EXPORT Q_DECL_IMPORT
#endif

#endif // SFTP_GLOBAL_H
