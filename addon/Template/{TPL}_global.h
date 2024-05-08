#ifndef {TPL}_GLOBAL_H
#define {TPL}_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined({TPL}_LIBRARY)
#  define {TPL}_EXPORT Q_DECL_EXPORT
#else
#  define {TPL}_EXPORT Q_DECL_IMPORT
#endif

#endif // {TPL}_GLOBAL_H
