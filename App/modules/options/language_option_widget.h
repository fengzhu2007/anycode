#ifndef LANGUAGE_OPTION_WIDGET_H
#define LANGUAGE_OPTION_WIDGET_H
#include "option_widget.h"

namespace Ui {
class LanguageOptionWidget;
}

namespace ady{
class LanguageOptionWidgetPrivate;
class LanguageOptionWidget : public OptionWidget
{
    Q_OBJECT
public:
    explicit LanguageOptionWidget(QWidget *parent = nullptr);
    virtual ~LanguageOptionWidget() override;

    virtual QString name() override;
    virtual void apply(int *state) override;
    virtual void initValue(const QJsonObject& value) override;
    virtual QJsonObject toJson() override;


    void initView();
private:
    Ui::LanguageOptionWidget* ui;
    LanguageOptionWidgetPrivate* d;
};
}
#endif // LANGUAGE_OPTION_WIDGET_H
