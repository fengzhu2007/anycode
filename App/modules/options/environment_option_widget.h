#ifndef ENVIRONMENT_OPTION_WIDGET_H
#define ENVIRONMENT_OPTION_WIDGET_H

#include "option_widget.h"

namespace Ui {
class EnvironmentOptionWidget;
}

namespace ady{

class EnvironmentOptionWidget : public OptionWidget
{
    Q_OBJECT

public:
    explicit EnvironmentOptionWidget(QWidget *parent = nullptr);
    virtual ~EnvironmentOptionWidget() override;

    virtual QString name() override;
    virtual void initValue(const QJsonObject& value) override;
    virtual QJsonObject toJson() override;

private:
    Ui::EnvironmentOptionWidget *ui;
};

}
#endif // ENVIRONMENT_OPTION_WIDGET_H
