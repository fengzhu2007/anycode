#ifndef PROXY_TAB_H
#define PROXY_TAB_H

#include "../option_tab.h"

namespace Ui{
class ProxyTab;
}

namespace ady{
class ProxyTab : public OptionTab
{
public:
    explicit ProxyTab(QWidget* parent);
    virtual ~ProxyTab();

    virtual QString name() override;
    virtual void apply(int *state) override;
    virtual void initValue(const QJsonObject& value) override;
    virtual QJsonObject toJson() override;

    void initView();

private:
    Ui::ProxyTab* ui;
};
}

#endif // PROXY_TAB_H
