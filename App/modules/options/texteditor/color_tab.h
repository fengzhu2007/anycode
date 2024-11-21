#ifndef COLOR_TAB_H
#define COLOR_TAB_H

#include "../option_tab.h"

namespace Ui {
class ColorTab;
}
namespace ady{

class ColorTab : public OptionTab
{
    Q_OBJECT

public:
    explicit ColorTab(QWidget *parent = nullptr);
    ~ColorTab();

    virtual QString name() override;
    virtual void initValue(const QJsonObject& value) override;
    virtual QJsonObject toJson() override;

    void initView();
private:
    Ui::ColorTab *ui;
};
}
#endif // COLOR_TAB_H
