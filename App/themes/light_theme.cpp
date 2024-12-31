#include "light_theme.h"

namespace ady{
class LightThemePrivate{
public:
    QColor color;
    QColor primaryColor;
    QColor secondaryColor;
    QColor backgroundColor;
    QColor textColor;
    QColor primaryTextColor;
    QColor secondaryTextColor;

    LightThemePrivate():
        color("#EEEEF2"),
        primaryColor("#007acc"),
        secondaryColor("#c9def5"),
        backgroundColor(Qt::white),
        textColor(Qt::black),
        primaryTextColor(Qt::white),
        secondaryTextColor(Qt::gray)
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

QColor LightTheme::textColor(){
    return d->textColor;
}

QColor LightTheme::primaryTextColor(){
    return d->primaryTextColor;
}

QColor LightTheme::secondaryTextColor(){
    return d->secondaryTextColor;
}

DockingTheme* LightTheme::docking(){
    return nullptr;
}

}
