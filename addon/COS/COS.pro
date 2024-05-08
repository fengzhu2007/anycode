QT = widgets sql

TEMPLATE = lib
DEFINES += COS_LIBRARY

CONFIG += c++11

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
    COSListObjectResponse.cpp \
    Export.cpp \
    COSFormDirSetting.cpp \
    COSFormGeneral.cpp \
    COSPanel.cpp \
    COSParse.cpp \
    COSResponse.cpp \
    COSSetting.cpp \
    COSWorker.cpp \
    cos.cpp

HEADERS += \
    COSListObjectResponse.h \
    Export.h \
    COSFormDirSetting.h \
    COSFormGeneral.h \
    COSPanel.h \
    COSParse.h \
    COSResponse.h \
    COSSetting.h \
    COSSettingKey.h \
    COSWorker.h \
    COS_global.h \
    cos.h



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
    COSFormDirSetting.ui \
    COSFormGeneral.ui \
    COSPanelUi.ui



TRANSLATIONS = \
    language.zh_CN.ts
