#ifndef LIBSVNPP_GLOBAL_H
#define LIBSVNPP_GLOBAL_H

//#include <QtCore/qglobal.h>
#if defined(_WIN32)

#if defined(LIBSVNPP_LIBRARY)
#  define LIBSVNPP_EXPORT __declspec(dllexport)
#else
#  define LIBSVNPP_EXPORT __declspec(dllimport)
#endif

#else

#   define LIBSVNPP_EXPORT

#endif

#endif // LIBSVNPP_GLOBAL_H
