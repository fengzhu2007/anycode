#include "dark_theme.h"
#include <QPalette>
namespace ady{

class DarkThemePrivate{
public:
    const QColor color;
    const QColor primaryColor;
    const QColor secondaryColor;
    const QColor backgroundColor;
    const QColor textColor;
    const QColor primaryTextColor;
    const QColor secondaryTextColor;
    const QColor borderColor;

    DarkThemePrivate():
        color("#2d2d30"),
        primaryColor("#007acc"),
        secondaryColor("#494949"),
        backgroundColor("#252526"),
        textColor("#efefef"),
        primaryTextColor(Qt::white),
        secondaryTextColor(Qt::gray),
        borderColor("#494949")
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

QColor DarkTheme::borderColor(){
    return d->borderColor;
}


DockingTheme* DarkTheme::docking(){
    return nullptr;
}

void DarkTheme::setup(QApplication& app){
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window,d->color);
    darkPalette.setColor(QPalette::WindowText, d->textColor);
    darkPalette.setColor(QPalette::Base, d->color);
    darkPalette.setColor(QPalette::AlternateBase, d->secondaryColor);
    darkPalette.setColor(QPalette::ToolTipBase, d->color);
    darkPalette.setColor(QPalette::ToolTipText, d->textColor);
    darkPalette.setColor(QPalette::Text, d->textColor);
    darkPalette.setColor(QPalette::Button,Qt::black);
    darkPalette.setColor(QPalette::ButtonText, d->textColor);
    darkPalette.setColor(QPalette::BrightText, Qt::white);
    darkPalette.setColor(QPalette::Link, d->primaryColor);

    darkPalette.setColor(QPalette::Highlight, d->textColor);
    darkPalette.setColor(QPalette::HighlightedText, Qt::white);

    darkPalette.setColor(QPalette::Light,Qt::black);
    darkPalette.setColor(QPalette::Midlight,Qt::black);

    darkPalette.setColor(QPalette::Dark,Qt::black);
    darkPalette.setColor(QPalette::Mid,Qt::black);
    darkPalette.setColor(QPalette::Shadow,Qt::black);
    darkPalette.setColor(QPalette::NoRole,Qt::black);
    darkPalette.setColor(QPalette::PlaceholderText,Qt::lightGray);


    //expand

    app.setPalette(darkPalette);
}

QString DarkTheme::qss(){
    auto backgroundColor = d->backgroundColor.name(QColor::HexRgb);
    auto textColor = d->textColor.name(QColor::HexRgb);
    auto color = d->color.name(QColor::HexRgb);
    auto secondaryColor = d->secondaryColor.name(QColor::HexRgb);
    auto primaryColor = d->primaryColor.name(QColor::HexRgb);
    return (".QStatusBar{background: #007acc;}"
                             ".QStatusBar::item{border:0;}"
                            ".QPushButton{background:#3e3e40;border:1px solid #ccc;padding:4px 6px;margin:1px 6px;min-width:60px;}"
                            ".QPushButton:hover{border-color:"+primaryColor+"}"
                             ".QPushButton:pressed{background:"+primaryColor+";color:white;}"
                             "QGroupBox{color:"+textColor+";border:1px solid "+secondaryColor+";margin-top:2ex;padding-top:8px;}"
                            " QGroupBox::title {subcontrol-origin: margin;subcontrol-position: top left;left:10px;}"
                            "QTabBar::tab{background:"+backgroundColor+";}"
                                "QTabBar::tab:selected, QTabBar::tab:hover{background:"+secondaryColor+";}"
                               "QTabWidget::pane{border: 1px solid "+secondaryColor+";}"
                            "QComboBox,QLineEdit{background:#333337;border:1px solid "+secondaryColor+"}"
                               "QComboBox::drop-down {subcontrol-origin:padding;subcontrol-position:top right;background:"+secondaryColor+";border:1px solid "+secondaryColor+";}"
                               "QComboBox::down-arrow{image:url(:/Resource/icons/dark/GlyphDown_16x.svg);background-color:"+secondaryColor+";}"
                               "QComboBox::drop-down:hover{background-color:"+backgroundColor+";border-color:"+backgroundColor+";}"
                               "QComboBox::down-arrow:hover{image:url(:/Resource/icons/dark/GlyphDownHover_16x.svg);background-color:"+backgroundColor+";}"
                             ".ady--PasswordEdit,.QLineEdit{background:#333337;height:24px;border:1px solid "+secondaryColor+";font-size:12px;}"
                             ".ady--PasswordEdit:focus,.QLineEdit:focus,.ady--PasswordEdit:hover,.QLineEdit:hover,.QComoboBox:focus,.QComoboBox:hover{border:1px solid #007acc}"
                               ".ady--PasswordEdit:disabled,QLineEdit:disabled,QComboBox:disabled,QPushButton:disabled{background:#555;color:#c3c3c3}"

#ifdef Q_OS_WIN
                             ".QSpinBox{border:1px solid "+secondaryColor+";font-size:12px;height:24px;}"
                             ".QSpinBox:hover{border:1px solid "+primaryColor+"}"
                             ".QSpinBox::up-button{background:"+secondaryColor+";}"
                             ".QSpinBox::up-button:hover{background:"+backgroundColor+";}"
                             ".QSpinBox::down-button{background:"+secondaryColor+";}"
                             ".QSpinBox::down-button:hover{background:"+backgroundColor+";}"
                             ".QSpinBox::up-arrow{image:url(:/Resource/icons/dark/GlyphUp_16x.svg);width:14px;height:14px;}"
                            ".QSpinBox::up-arrow:hover{image:url(:/Resource/icons/dark/GlyphUpHover_16x.svg);}"
                             ".QSpinBox::down-arrow{image:url(:/Resource/icons/dark/GlyphDown_16x.svg);width:14px;height:14px;}"
                                ".QSpinBox::down-arrow:hover{image:url(:/Resource/icons/dark/GlyphDownHover_16x.svg);}"
                             "QTreeView{border:0;background-color:"+backgroundColor+";color:"+textColor+";}"
                            "QTreeView::branch:has-children:open{image: url(':/Resource/icons/dark/Expand_16x.svg');border-image:none;}"
                            "QTreeView::branch:has-children:closed{image: url(':/Resource/icons/dark/Collapse_16x.svg');border-image:none;}"
                                                        "QTreeView::branch:has-children:open:hover{image: url(':/Resource/icons/dark/ExpandHover_16x.svg');}"
                                                        "QTreeView::branch:has-children:closed:hover{image: url(':/Resource/icons/dark/CollapseHover_16x.svg');}"


                        ".ady--ListView>QWidget#qt_scrollarea_viewport>.QWidget{background-color:"+color+";}"
                        "ady--ListViewItem[state='1']{background-color:"+secondaryColor+"}"
                        "ady--ListViewItem:hover{background-color:"+secondaryColor+"}"

                             "#widget{background-color:"+color+"}"
                            ".QHeaderView::section{background-color:#373738;color:white;border: 1px solid #373738;border-right:1px solid #3e3e40;border-bottom:1px solid #3e3e40;}"
                            ".QLabel,QRadioButton,QCheckBox{color:"+textColor+"}"
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
                            ".QComboBox{height:24px}"
                          ".QMessageBox .QLabel{color:black;}"
                          ".QMessageBox .QPushButton{background-color: none;color:black;}"
                          ".QToolBar::separator{background-color:"+secondaryColor+";width:1px;margin:4px 2px;}"
                          ".QToolButton{border:2px solid #00000000;background:none;}"
                          ".QToolButton:hover{background-color:"+secondaryColor+";}"
                        ".QToolButton:pressed,.QToolButton:checked{background:"+primaryColor+";}"
                             "QToolButton[popupMode='1'] {padding-right: 14px;margin-top:4px;}"
                             "QToolButton::menu-arrow{top:2px;}"
                             //texteditor
                             ".Utils--FakeToolTip{background-color:"+backgroundColor+"}"
                             "");
}

}
