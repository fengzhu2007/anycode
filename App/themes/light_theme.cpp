#include "light_theme.h"

namespace ady{
class LightThemePrivate{
public:
    const QColor color;
    const QColor primaryColor;
    const QColor secondaryColor;
    const QColor backgroundColor;
    const QColor secondaryBackgroundColor;
    const QColor textColor;
    const QColor primaryTextColor;
    const QColor secondaryTextColor;
    const QColor borderColor;

    LightThemePrivate():
        color("#EEEEF2"),
        primaryColor("#007acc"),
        secondaryColor("#c9def5"),
        backgroundColor("#f5f5f5"),
        secondaryBackgroundColor("#f5f5f5"),
        textColor(Qt::black),
        primaryTextColor(Qt::white),
        secondaryTextColor(Qt::gray),
        borderColor("#ccc")
    {

    }

};

LightTheme::LightTheme():Theme() {
    d = new LightThemePrivate;
}

LightTheme::~LightTheme(){
    delete d;
}

QString LightTheme::name(){
    return QString::fromUtf8("Light");
}

Theme::Style LightTheme::style(){
    return Theme::Light;
}

//
QColor LightTheme::color(){
    return d->color;
}

QColor LightTheme::primaryColor(){
    return d->primaryColor;
}

QColor LightTheme::secondaryColor(){
    return d->secondaryColor;
}

QColor LightTheme::backgroundColor(){
    return d->backgroundColor;
}

QColor LightTheme::secondaryBackgroundColor(){
    return d->secondaryBackgroundColor;
}

QColor LightTheme::textColor(){
    return d->textColor;
}

QColor LightTheme::primaryTextColor(){
    return d->primaryTextColor;
}

QColor LightTheme::secondaryTextColor(){
    return d->secondaryTextColor;
}

QColor LightTheme::borderColor(){
    return d->borderColor;
}

DockingTheme* LightTheme::docking(){
    return nullptr;
}
QString LightTheme::qss(){
    return QString::fromUtf8(".QStatusBar{background: #007acc;}"
                             ".QStatusBar::item{border:0;}"
                             ".ady--PasswordEdit,.QLineEdit{height:24px;border:1px solid #ccc;font-size:12px;}"
                             ".ady--PasswordEdit:focus,.QLineEdit:focus,.ady--PasswordEdit:hover,.QLineEdit:hover{border:1px solid #007acc}"
                             ".ady--ListView>QWidget#qt_scrollarea_viewport>.QWidget{background-color:white;}"
                             "ady--ListViewItem[state='1']{background-color:#d9e0f8}"
                             "ady--ListViewItem:hover{background-color:#c9def5}"
                             "QTreeView{border:0;background:#f5f5f5}"

#ifdef Q_OS_WIN
                             ".QSpinBox{border:1px solid #ccc;font-size:12px;height:24px;}"
                             ".QSpinBox:hover{border:1px solid #007acc}"
                             ".QSpinBox::up-button{background:#EEEEF2;}"
                             ".QSpinBox::up-button:hover{background:#c9def5;}"
                             ".QSpinBox::up-button:pressed{background:#007acc;}"
                             ".QSpinBox::down-button{background:#EEEEF2;}"
                             ".QSpinBox::down-button:hover{background:#c9def5;}"
                             ".QSpinBox::down-button:pressed{background:#007acc;}"
                             ".QSpinBox::up-arrow{image:url(:/Resource/icons/GlyphUp_16x.svg);width:14px;height:14px;}"
                             ".QSpinBox::down-arrow{image:url(:/Resource/icons/GlyphDown_16x.svg);width:14px;height:14px;}"
#else


#endif
                             ".QComboBox{height:24px}");
}

}
