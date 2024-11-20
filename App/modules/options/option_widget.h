#ifndef OPTION_WIDGET_H
#define OPTION_WIDGET_H

#include "global.h"
#include <QWidget>
#include <QJsonObject>


namespace ady{
class ANYENGINE_EXPORT OptionWidget : public QWidget
{
public:
    explicit OptionWidget(QWidget* parent);
    virtual ~OptionWidget();

    virtual QString name()=0;
    virtual void initValue(const QJsonObject& value);
    virtual QJsonObject toJson();





};

}

#endif // OPTION_WIDGET_H
