QT = widgets sql

TEMPLATE = lib
DEFINES += OSS_LIBRARY

CONFIG += c++11

QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
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
    Export.cpp \
    OSSFormDirSetting.cpp \
    OSSFormGeneral.cpp \
    OSSPanel.cpp \
    OSSResponse.cpp \
    OSSSetting.cpp \
    OSSWorker.cpp \
    oss.cpp

HEADERS += \
    Export.h \
    OSSFormDirSetting.h \
    OSSFormGeneral.h \
    OSSPanel.h \
    OSSResponse.h \
    OSSSetting.h \
    OSSSettingKey.h \
    OSSWorker.h \
    OSS_global.h \
    oss.h



INCLUDEPATH += $$PWD/../../ui/App

INCLUDEPATH += $$PWD/../../include

win32: INCLUDEPATH += $$PWD\..\..\platforms\win32

unix: INCLUDEPATH += $$PWD/../../platforms/unix

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../ui/App/release/ -lApp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../ui/App/debug/ -lApp
else:unix: LIBS += -L$$OUT_PWD/../../ui/App/ -lApp


win32: LIBS += -L$$PWD\..\..\libs\openssl -llibcrypto
win32: LIBS += -L$$PWD\..\..\libs\openssl -llibssl

unix: LIBS += -L$$PWD/../../third_party/build/openssl-1.1.1l/lib -lcrypto
unix: LIBS += -L$$PWD/../../third_party/build/openssl-1.1.1l/lib -lssl

CONFIG(debug, debug|release) {
    DEFINES += _DEBUG
    win32: LIBS += -L$$PWD\..\..\libs\curl -llibcurl-d_imp
    unix: LIBS += -L$$PWD/../../third_party/build/curl-7.83.1/lib -lcurl

}else{
    DEFINES += NDEBUG
     win32: LIBS += -L$$PWD\..\..\libs\curl -llibcurl_imp
    unix: LIBS += -L$$PWD/../../third_party/build/curl-7.83.1/lib -lcurl
}



# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

FORMS += \
    OSSFormDirSetting.ui \
    OSSFormGeneral.ui \
    OSSPanelUi.ui

TRANSLATIONS = \
    language.zh_CN.ts
