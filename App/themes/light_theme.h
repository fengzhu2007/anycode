#ifndef LIGHT_THEME_H
#define LIGHT_THEME_H

#include "core/theme.h"

namespace ady{
class LightThemePrivate;
class LightTheme : public Theme
{
public:
    virtual ~LightTheme()  override;
    virtual QString name() override;
    virtual Style style() override;
    virtual QColor color()  override;//default color
    virtual QColor primaryColor()  override;
    virtual QColor secondaryColor()  override;
    virtual QColor backgroundColor()  override;

    virtual QColor textColor()  override;
    virtual QColor primaryTextColor()  override;
    virtual QColor secondaryTextColor()  override;

    virtual DockingTheme* docking() override;
private:
    LightTheme();


private:
    LightThemePrivate* d;
    friend class Theme;
};
}

#endif // LIGHT_THEME_H
