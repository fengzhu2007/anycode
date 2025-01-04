#include "dark_theme.h"

namespace ady{

class DarkThemePrivate{
public:
    QColor color;
    QColor primaryColor;
    QColor secondaryColor;
    QColor backgroundColor;
    QColor textColor;
    QColor primaryTextColor;
    QColor secondaryTextColor;

    DarkThemePrivate():
        color("#2d2d30"),
        primaryColor("#007acc"),
        secondaryColor("#494949"),
        backgroundColor("#252526"),
        textColor(Qt::white),
        primaryTextColor(Qt::white),
        secondaryTextColor(Qt::gray)
    {

    }

};

DarkTheme::DarkTheme():Theme() {
    d = new DarkThemePrivate;
}

DarkTheme::~DarkTheme(){
    delete d;
}

QString DarkTheme::name(){
    return QString::fromUtf8("Dark");
}

Theme::Style DarkTheme::style(){
    return Theme::Dark;
}

//
QColor DarkTheme::color(){
    return d->color;
}

QColor DarkTheme::primaryColor(){
    return d->primaryColor;
}

QColor DarkTheme::secondaryColor(){
    return d->secondaryColor;
}

QColor DarkTheme::backgroundColor(){
    return d->backgroundColor;
}

QColor DarkTheme::textColor(){
    return d->textColor;
}

QColor DarkTheme::primaryTextColor(){
    return d->primaryTextColor;
}

QColor DarkTheme::secondaryTextColor(){
    return d->secondaryTextColor;
}

DockingTheme* DarkTheme::docking(){
    return nullptr;
}

QString DarkTheme::qss(){
    auto backgroundColor = d->backgroundColor.name(QColor::HexRgb);
    auto textColor = d->textColor.name(QColor::HexRgb);
    auto color = d->color.name(QColor::HexRgb);
    return (".QStatusBar{background: #007acc;}"
                             ".QStatusBar::item{border:0;}"
                             ".ady--PasswordEdit,.QLineEdit{height:24px;border:1px solid #ccc;font-size:12px;}"
                             ".ady--PasswordEdit:focus,.QLineEdit:focus,.ady--PasswordEdit:hover,.QLineEdit:hover{border:1px solid #007acc}"

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
                             ".QTreeView{border:0;background-color:"+backgroundColor+";color:"+textColor+";}"
                             "#widget{background-color:"+color+"}"
                            ".QHeaderView::section{background-color:#494949;color:white;border: 1px solid #494949;border-right:1px solid "+backgroundColor+";}"
                                ".QLabel{color:"+textColor+"}"
                          ".QScrollBar:horizontal{border:1px solid grey; background: "+backgroundColor+";height: 18px;margin: 0px 20px 0 20px;}"
                          ".QScrollBar::handle:horizontal{background: #494949;min-width: 20px;}"
                          ".QScrollBar::add-line:horizontal{border: 1px solid grey;background: "+backgroundColor+";width: 20px;subcontrol-position:right;subcontrol-origin: margin;}"
                          ".QScrollBar::sub-line:horizontal{border: 1px solid grey;background: "+backgroundColor+";width: 20px;subcontrol-position:left;subcontrol-origin: margin;}"
                                ".QScrollBar:vertical{border:1px solid grey; background: "+backgroundColor+";width: 18px;margin:20px 0 20px  0px ;}"
                                ".QScrollBar::handle:vertical{background: #494949;min-height: 20px;}"
                                ".QScrollBar::add-line:vertical{border: 1px solid grey;background: "+backgroundColor+";height: 20px;subcontrol-position:right;subcontrol-origin: margin;}"
                                ".QScrollBar::sub-line:vertical{border: 1px solid grey;background: "+backgroundColor+";height: 20px;subcontrol-position:left;subcontrol-origin: margin;}"
#else


#endif
                             ".QComboBox{height:24px}");
}

}
