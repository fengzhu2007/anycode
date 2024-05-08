QT = gui core

TEMPLATE = lib
DEFINES += LIBSVNPP_LIBRARY

CONFIG += c++11

QMAKE_CXXFLAGS_RELEASE = -O2 -MD -GL
QMAKE_CXXFLAGS_DEBUG = -Zi -MDd

QMAKE_LFLAGS_DEBUG      = /DEBUG /NODEFAULTLIB:libc.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:libcd.lib /NODEFAULTLIB:libcmtd.lib
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/client/diff_summary.cpp \
    src/client/info.cpp \
    src/client/log_entry.cpp \
    src/svnpp_client.cpp

HEADERS += \
    include/client/diff_summary.h \
    include/client/info.h \
    include/client/log_entry.h \
    include/error.h \
    include/svnpp_callback.h \
    include/svnpp_global.h \
    include/svnpp_client.h \
    include/types.h

INCLUDEPATH += $$PWD/include
INCLUDEPATH += $$PWD/../third_party/libsvn/subversion-1.14.2/subversion/include
unix:INCLUDEPATH += $$PWD/../third_party/libsvn/apr-1.7.0/include
unix:INCLUDEPATH += $$PWD/../third_party/libsvn/apr-util-1.6.1/include
win32:INCLUDEPATH += $$PWD/../third_party/libsvn/apr-1.7.0/include
win32:INCLUDEPATH += $$PWD/../third_party/libsvn/apr-util-1.6.1/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../third_party/libsvn/bin/win32/lib/ -lzlibstatic
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../third_party/libsvn/bin/win32/lib/ -lzlibstatic

DEPENDPATH += $$PWD/../third_party/libsvn/bin/win32/include



win32: LIBS += ws2_32.lib rpcrt4.lib mswsock.lib kernel32.lib advapi32.lib shfolder.lib ole32.lib crypt32.lib version.lib

CONFIG(debug, debug|release) {
    DEFINES += _DEBUG
    win32: LIBS += -L$$PWD\..\libs\openssl -llibcrypto
    win32: LIBS += -L$$PWD\..\libs\openssl -llibssl
    win32: LIBS += -L$$PWD\..\third_party\libsvn\apr-1.7.0\Release -llibapr-1
    win32: LIBS += -L$$PWD\..\third_party\libsvn\apr-util-1.6.1\Release -llibaprutil-1
    win32: LIBS += -L$$PWD\..\third_party\libsvn\apr-util-1.6.1\Release -llibexpat
    #win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_subr -llibsvn_subr-1
    #win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_delta -llibsvn_delta-1
    #win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_client -llibsvn_client-1
    #win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_ra -llibsvn_ra-1
    #win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_diff -llibsvn_diff-1
    #win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_fs -llibsvn_fs-1
    #win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_fs_fs -llibsvn_fs_fs-1
    #win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_fs_util -llibsvn_fs_util-1
    #win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_fs_x -llibsvn_fs_x-1
    #win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_wc -llibsvn_wc-1

}else{
    DEFINES += NDEBUG
    win32: LIBS += -L$$PWD\..\libs\openssl -llibcrypto
    win32: LIBS += -L$$PWD\..\libs\openssl -llibssl
    win32: LIBS += -L$$PWD\..\third_party\libsvn\apr-1.7.0\Release -llibapr-1
    win32: LIBS += -L$$PWD\..\third_party\libsvn\apr-util-1.6.1\Release -llibaprutil-1
    win32: LIBS += -L$$PWD\..\third_party\libsvn\apr-util-1.6.1\Release -llibexpat
    #win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_subr -llibsvn_subr-1
    #win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_delta -llibsvn_delta-1
    #win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_client -llibsvn_client-1
    #win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_ra -llibsvn_ra-1
    #win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_diff -llibsvn_diff-1
    #win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_fs -llibsvn_fs-1
    #win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_fs_fs -llibsvn_fs_fs-1
    #win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_fs_util -llibsvn_fs_util-1
    #win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_fs_x -llibsvn_fs_x-1
    #win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_wc -llibsvn_wc-1
}

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_subr -lsvn_subr-1
win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_delta -lsvn_delta-1
win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_client -lsvn_client-1
win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_ra -lsvn_ra-1
win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_diff -lsvn_diff-1
win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_fs -lsvn_fs-1
win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_fs_fs -lsvn_fs_fs-1
win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_fs_util -lsvn_fs_util-1
win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_fs_x -lsvn_fs_x-1
win32: LIBS += -L$$PWD\..\third_party\libsvn\subversion-1.14.2\Release\subversion\libsvn_wc -lsvn_wc-1



win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_subr/svn_subr-1.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_subr/svn_subr-1.a

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_delta/svn_delta-1.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_delta/svn_delta-1.a

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_client/svn_client-1.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_client/libsvn_client-1.a

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_ra/svn_ra-1.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_ra/svn_ra-1.a

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_diff/svn_diff-1.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_diff/svn_diff-1.a

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_fs/svn_fs-1.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_fs/svn_fs-1.a

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_fs_fs/svn_fs_fs-1.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_fs_fs/svn_fs_fs-1.a

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_fs_util/svn_fs_util-1.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_fs_util/svn_fs_util-1.a

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_fs_x/svn_fs_x-1.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_fs_x/svn_fs_x-1.a

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_wc/svn_wc-1.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_wc/svn_wc-1.a


DEPENDPATH += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_subr
DEPENDPATH += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_delta
DEPENDPATH += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_client
DEPENDPATH += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_ra
DEPENDPATH += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_diff
DEPENDPATH += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_fs
DEPENDPATH += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_fs_fs
DEPENDPATH += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_fs_util
DEPENDPATH += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_fs_x
DEPENDPATH += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_wc



win32: LIBS += -L$$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_ra_svn/ -llibsvn_ra_svn-1

DEPENDPATH += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_ra_svn

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_ra_svn/libsvn_ra_svn-1.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_ra_svn/liblibsvn_ra_svn-1.a

win32: LIBS += -L$$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_ra_local/ -llibsvn_ra_local-1

INCLUDEPATH += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_ra_local
DEPENDPATH += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_ra_local

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_ra_local/libsvn_ra_local-1.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_ra_local/liblibsvn_ra_local-1.a

win32: LIBS += -L$$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_repos/ -lsvn_repos-1

INCLUDEPATH += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_repos
DEPENDPATH += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_repos

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_repos/svn_repos-1.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/subversion-1.14.2/Release/subversion/libsvn_repos/libsvn_repos-1.a





macx: LIBS += -L$$PWD/../third_party/libsvn/build/apr-1.7.0/lib/ -lapr-1
macx: LIBS += -L$$PWD/../third_party/libsvn/build/apr-util-1.6.1/lib/ -laprutil-1


macx: LIBS += -L$$PWD/../third_party/libsvn/build/subversion-1.14.2/lib/ -lsvn_client-1
macx: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/build/subversion-1.14.2/lib/libsvn_client-1.a

macx: LIBS += -L$$PWD/../third_party/libsvn/build/subversion-1.14.2/lib/ -lsvn_ra_svn-1
macx: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/build/subversion-1.14.2/lib/libsvn_ra_svn-1.a

macx: LIBS += -L$$PWD/../third_party/libsvn/build/subversion-1.14.2/lib/ -lsvn_ra_local-1
macx: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/build/subversion-1.14.2/lib/libsvn_ra_local-1.a

macx: LIBS += -L$$PWD/../third_party/libsvn/build/subversion-1.14.2/lib/ -lsvn_repos-1
macx: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/build/subversion-1.14.2/lib/libsvn_repos-1.a

macx: LIBS += -L$$PWD/../third_party/libsvn/build/subversion-1.14.2/lib/ -lsvn_subr-1
macx: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/build/subversion-1.14.2/lib/libsvn_subr-1.a

macx: LIBS += -L$$PWD/../third_party/libsvn/build/subversion-1.14.2/lib/ -lsvn_ra-1
macx: PRE_TARGETDEPS += $$PWD/../third_party/libsvn/build/subversion-1.14.2/lib/libsvn_ra-1.a

