#ifndef GENERAL_TAB_H
#define GENERAL_TAB_H

#include "../option_tab.h"
namespace Ui{
class GeneralTab;

}

namespace ady{
class GeneralTab : public OptionTab
{
public:
    explicit GeneralTab(QWidget* parent);
    virtual ~GeneralTab();

    virtual QString name() override;
    virtual void initValue(const QJsonObject& value) override;
    virtual QJsonObject toJson() override;

    void initView();

private:
    Ui::GeneralTab* ui;
};
}

#endif // GENERAL_TAB_H
