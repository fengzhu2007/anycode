#ifndef THEME_H
#define THEME_H
#include <QColor>
#include <QString>
#include <QApplication>
#include "global.h"

//#include <docking_theme.h>
namespace ady{
class DockingTheme;
class ANYENGINE_EXPORT Theme
{
public:
    enum Style{
        Light=0,
        Dark
    };
    virtual ~Theme();
    virtual QString name()=0;
    virtual Style style()=0;
    virtual QColor color()=0;//default color
    virtual QColor primaryColor()=0;
    virtual QColor secondaryColor()=0;
    virtual QColor backgroundColor()=0;

    virtual QColor textColor()=0;
    virtual QColor primaryTextColor()=0;
    virtual QColor secondaryTextColor()=0;

    virtual QColor borderColor()=0;

    virtual DockingTheme* docking()=0;

    virtual QString qss()=0;

    virtual void setup(QApplication& app);

    template<class T>
    static Theme* init(){
        destory();//delete before
        instance = new T;
        return instance;
    }
    static void destory();
    static Theme* getInstance(){
        return instance;
    }

protected:
    Theme();

private:
    static Theme* instance;

};
}
#endif // THEME_H
