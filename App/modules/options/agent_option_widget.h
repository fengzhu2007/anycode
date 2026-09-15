#ifndef AGENT_OPTION_WIDGET_H
#define AGENT_OPTION_WIDGET_H

#include "option_widget.h"

namespace Ui {
class AgentOptionWidget;
}

namespace ady{
class AgentOptionWidgetPrivate;
class AgentOptionWidget : public OptionWidget
{
    Q_OBJECT

public:
    explicit AgentOptionWidget(QWidget *parent = nullptr);
    virtual ~AgentOptionWidget() override;

    virtual QString name() override;
    virtual void apply(int *state) override;
    virtual void initValue(const QJsonObject& value) override;
    virtual QJsonObject toJson() override;

    void initView();

private:
    Ui::AgentOptionWidget *ui;
    AgentOptionWidgetPrivate* d;
};
}

#endif // AGENT_OPTION_WIDGET_H
