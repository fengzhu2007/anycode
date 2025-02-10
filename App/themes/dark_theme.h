#ifndef DARK_THEME_H
#define DARK_THEME_H

#include "core/theme.h"
namespace ady{
class DarkThemePrivate;
class ANYENGINE_EXPORT DarkTheme : public Theme
{
public:
    virtual ~DarkTheme()  override;
    virtual QString name() override;
    virtual Style style() override;
    virtual QColor color()  override;//default color
    virtual QColor primaryColor()  override;
    virtual QColor secondaryColor()  override;
    virtual QColor backgroundColor()  override;
    virtual QColor secondaryBackgroundColor()override;

    virtual QColor textColor()  override;
    virtual QColor primaryTextColor()  override;
    virtual QColor secondaryTextColor()  override;

    virtual QColor borderColor() override;

    virtual DockingTheme* docking() override;

    virtual void setup(QApplication& app) override;
    virtual QString qss() override;


private:
    DarkTheme();


private:
    DarkThemePrivate* d;
    friend class Theme;
};
}
#endif // DARK_THEME_H
