#ifndef AI_OPTION_WIDGET_H
#define AI_OPTION_WIDGET_H

#include "option_widget.h"

namespace Ui {
class AIOptionWidget;
}

namespace ady{

class AIOptionWidget : public OptionWidget
{
    Q_OBJECT

public:
    explicit AIOptionWidget(QWidget *parent = nullptr);
    virtual ~AIOptionWidget() override;

    virtual QString name() override;
    virtual void apply(int *state) override;
    virtual void initValue(const QJsonObject& value) override;
    virtual QJsonObject toJson() override;


    void initView();
private:
    Ui::AIOptionWidget *ui;
};

}

#endif // AI_OPTION_WIDGET_H
