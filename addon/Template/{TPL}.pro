QT = widgets sql

TEMPLATE = lib
DEFINES += {TPL}_LIBRARY

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
    ChmodDialog.cpp \
    Export.cpp \
    {TPL}FormDirSetting.cpp \
    {TPL}FormGeneral.cpp \
    {TPL}Panel.cpp \
    {TPL}Parse.cpp \
    {TPL}Response.cpp \
    {TPL}Setting.cpp \
    {TPL}Worker.cpp \
    {tpl}.cpp

HEADERS += \
    ChmodDialog.h \
    Export.h \
    {TPL}FormDirSetting.h \
    {TPL}FormGeneral.h \
    {TPL}Panel.h \
    {TPL}Parse.h \
    {TPL}Response.h \
    {TPL}Setting.h \
    {TPL}SettingKey.h \
    {TPL}Worker.h \
    {TPL}_global.h \
    {tpl}.h



INCLUDEPATH += $$PWD/../../ui/App

INCLUDEPATH += $$PWD/../../include

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../ui/App/release/ -lApp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../ui/App/debug/ -lApp
else:unix: LIBS += -L$$OUT_PWD/../../ui/App/ -lApp

CONFIG(debug, debug|release) {
    DEFINES += _DEBUG
    win32: LIBS += -L$$PWD\..\..\libs\curl -llibcurl-d_imp

    unix: LIBS += -L$$PWD/../../libs/curl -lcurl

}else{
    DEFINES += NDEBUG
    win32: LIBS += -L$$PWD\..\..\libs\curl -llibcurl

    unix: LIBS += -L$$PWD/../../libs/curl -lcurl
}

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

FORMS += \
    ChmodDialog.ui \
    {TPL}FormDirSetting.ui \
    {TPL}FormGeneral.ui \
    {TPL}PanelUi.ui



