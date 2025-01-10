#ifndef COLOR_TAB_H
#define COLOR_TAB_H

#include "../option_tab.h"

namespace Ui {
class ColorTab;
}
namespace ady{
class ColorTabPrivate;
class ColorTab : public OptionTab
{
    Q_OBJECT

public:
    explicit ColorTab(QWidget *parent = nullptr);
    ~ColorTab();

    virtual QString name() override;
    virtual void apply(int *state) override;
    virtual void initValue(const QJsonObject& value) override;
    virtual QJsonObject toJson() override;
    virtual void notifyChanged(const QString& name,const QVariant& value) override;

    void initView();

private:
    void resetThemeList(const QString& value={});


public slots:
    void onFontChanged(const QFont& font);
    void onValueChanged(int value);
    void onSchemeChanged(int index);


private:
    Ui::ColorTab *ui;
    ColorTabPrivate* d;

public:
    static const QString themeNameKey;
};
}
#endif // COLOR_TAB_H
