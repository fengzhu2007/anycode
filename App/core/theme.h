#ifndef THEME_H
#define THEME_H
#include <QColor>
#include <QString>
//#include <docking_theme.h>
namespace ady{
class DockingTheme;
class Theme
{
public:
    enum Style{
        Light,
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

    virtual DockingTheme* docking()=0;

    template<class T>
    T* init(){
        destory();//delete before
        instance = new T;
        return instance;
    }
    static void destory();

protected:
    Theme();

private:
    static Theme* instance;

};
}
#endif // THEME_H
