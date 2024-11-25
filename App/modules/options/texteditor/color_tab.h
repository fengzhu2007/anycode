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
    virtual void apply() override;
    virtual void initValue(const QJsonObject& value) override;
    virtual QJsonObject toJson() override;

    void initView();


public slots:
    void onFontChanged(const QFont& font);
    void onValueChanged(int value);
    void onSchemeChanged(int index);


private:
    Ui::ColorTab *ui;
    ColorTabPrivate* d;
};
}
#endif // COLOR_TAB_H
