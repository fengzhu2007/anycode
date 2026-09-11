#ifndef NETWORK_OPTION_WIDGET_H
#define NETWORK_OPTION_WIDGET_H

#include "option_widget.h"

namespace Ui {
class NetworkOptionWidget;
}

namespace ady{
class NetworkOptionWidgetPrivate;
class NetworkOptionWidget : public OptionWidget
{
    Q_OBJECT

public:
    explicit NetworkOptionWidget(QWidget *parent = nullptr);
    virtual ~NetworkOptionWidget() override;

    virtual QString name() override;
    virtual void apply(int *state) override;
    virtual void initValue(const QJsonObject& value) override;
    virtual QJsonObject toJson() override;

    void initView();

private:
    Ui::NetworkOptionWidget *ui;
    NetworkOptionWidgetPrivate* d;
};
}

#endif // NETWORK_OPTION_WIDGET_H
