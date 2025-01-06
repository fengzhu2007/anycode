#include "dark_theme.h"
#include <QPalette>
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

void DarkTheme::setup(QApplication& app){
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window,d->color);
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, d->color);
    darkPalette.setColor(QPalette::AlternateBase, d->secondaryColor);
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button,d->color);
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::white);
    darkPalette.setColor(QPalette::Link, d->primaryColor);

    darkPalette.setColor(QPalette::Highlight, d->primaryColor);
    darkPalette.setColor(QPalette::HighlightedText, Qt::white);

    //expand

    app.setPalette(darkPalette);
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
                            ".QTreeView::branch:has-children:open{image: url(':/Resource/icons/dark/ExpandDown_16x.svg');width:12px;height:12px;border-image:none;}"
                            ".QTreeView::branch:has-children:closed{image: url(':/Resource/icons/dark/CollapseLeft_16x.svg');border-image:none;}"






                             "#widget{background-color:"+color+"}"
                            ".QHeaderView::section{background-color:#373738;color:white;border: 1px solid #373738;border-right:1px solid #3e3e40;border-bottom:1px solid #3e3e40;}"
                            ".QLabel{color:"+textColor+"}"
                          ".QScrollBar:horizontal{border-top:4px solid #3e3e42;border-bottom:4px solid #3e3e42; background-color: #3e3e42;height: 18px;margin: 0px 20px 0 20px;}"
                          ".QScrollBar::handle:horizontal{background: #686868;min-width: 20px;}"
                          ".QScrollBar::add-line:horizontal{border: 1px solid #3e3e42;background: #3e3e42;width: 20px;subcontrol-position:right;subcontrol-origin: margin;image:url(':/Resource/icons/dark/GlyphRight_16x.svg');}"
                          ".QScrollBar::add-line:horizontal:hover{image:url(':/Resource/icons/dark/GlyphRightHover_16x.svg');}"
                          ".QScrollBar::add-line:horizontal:pressed{image:url(':/Resource/icons/dark/GlyphRightActive_16x.svg');}"

                          ".QScrollBar::sub-line:horizontal{border: 1px solid #3e3e42;background: #3e3e42;width: 20px;subcontrol-position:left;subcontrol-origin: margin;image:url(':/Resource/icons/dark/GlyphLeft_16x.svg');}"
                          ".QScrollBar::sub-line:horizontal:hover{image:url(':/Resource/icons/dark/GlyphLeftHover_16x.svg');}"
                          ".QScrollBar::sub-line:horizontal:pressed{image:url(':/Resource/icons/dark/GlyphLeftActive_16x.svg');}"

                            ".QScrollBar:vertical{border-left:4px solid #3e3e42;border-right:4px solid #3e3e42; background-color: #3e3e42;width: 18px;margin:20px 0 20px  0px ;}"
                            ".QScrollBar::handle:vertical{background: #686868;min-height: 20px;}"
                            ".QScrollBar::sub-page,.QScrollBar::add-page{background-color:#3e3e42}"
                            ".QScrollBar::add-line:vertical{border: 1px solid #3e3e42;background: #3e3e42;height: 20px;subcontrol-position:bottom;subcontrol-origin: margin;image:url(':/Resource/icons/dark/GlyphDown_16x.svg');}"
                          ".QScrollBar::add-line:vertical:hover{image:url(':/Resource/icons/dark/GlyphDownHover_16x.svg');}"
                          ".QScrollBar::add-line:vertical:pressed{image:url(':/Resource/icons/dark/GlyphDownActive_16x.svg');}"


                            ".QScrollBar::sub-line:vertical{border: 1px solid #3e3e42;background: #3e3e42;height: 20px;subcontrol-position:top;subcontrol-origin: margin;image:url(':/Resource/icons/dark/GlyphUp_16x.svg');}"
                          ".QScrollBar::sub-line:vertical:hover{image:url(':/Resource/icons/dark/GlyphUpHover_16x.svg');}"
                          ".QScrollBar::sub-line:vertical:pressed{image:url(':/Resource/icons/dark/GlyphUpActive_16x.svg');}"



                            ".QScrollBar::handle:hover{background-color:#9e9e9e;}"
                            ".QScrollBar::handle:pressed{background-color:#efefef;}"
                          ".QWidget#infoBar{background:#3e3e42}"
#else


#endif
                             ".QComboBox{height:24px}");
}

}
