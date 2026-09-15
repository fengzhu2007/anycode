#ifndef LOCAL_TAB_H
#define LOCAL_TAB_H

#include "../option_tab.h"

namespace Ui{
class LocalTab;
}

namespace ady{
class LocalTab : public OptionTab
{
public:
    explicit LocalTab(QWidget* parent);
    virtual ~LocalTab();

    virtual QString name() override;
    virtual void apply(int *state) override;
    virtual void initValue(const QJsonObject& value) override;
    virtual QJsonObject toJson() override;

    void initView();

private:
    Ui::LocalTab* ui;
};
}

#endif // LOCAL_TAB_H
