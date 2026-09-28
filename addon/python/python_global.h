#ifndef PYTHON_GLOBAL_H
#define PYTHON_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(PYTHON_LIBRARY)
#define PYTHON_EXPORT Q_DECL_EXPORT
#else
#define PYTHON_EXPORT Q_DECL_IMPORT
#endif

#endif // PYTHON_GLOBAL_H
