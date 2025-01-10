#ifndef TEXTEDITOR_OPTION_WIDGET_H
#define TEXTEDITOR_OPTION_WIDGET_H

#include "option_widget.h"

namespace Ui {
class TextEditorOptionWidget;
}


namespace ady{
class TextEditorOptionWidgetPrivate;
class TextEditorOptionWidget : public OptionWidget
{
    Q_OBJECT

public:
    explicit TextEditorOptionWidget(QWidget *parent = nullptr);
    virtual ~TextEditorOptionWidget();


    virtual QString name() override;
    virtual void apply(int *state) override;
    virtual void initValue(const QJsonObject& value) override;
    virtual QJsonObject toJson() override;
    virtual void notifyChanged(const QString& name,const QVariant& value) override;

    void initView();

private:
    Ui::TextEditorOptionWidget *ui;
    TextEditorOptionWidgetPrivate* d;
};
}

#endif // TEXTEDITOR_OPTION_WIDGET_H
