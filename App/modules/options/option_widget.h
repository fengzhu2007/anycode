#ifndef OPTION_WIDGET_H
#define OPTION_WIDGET_H

#include "global.h"
#include <QWidget>
#include <QJsonObject>


namespace ady{
class ANYENGINE_EXPORT OptionWidget : public QWidget
{
public:
    enum State{
        None=0,
        Restart=1
    };
    explicit OptionWidget(QWidget* parent);
    virtual ~OptionWidget();

    virtual QString name()=0;
    virtual void apply(int *state);
    virtual void initValue(const QJsonObject& value);
    virtual QJsonObject toJson();
    virtual void notifyChanged(const QString& name,const QVariant& value);





};

}

#endif // OPTION_WIDGET_H
