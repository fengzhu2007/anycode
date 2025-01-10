#ifndef OPTION_TAB_H
#define OPTION_TAB_H
#include <QWidget>
#include <QJsonObject>
namespace ady{
class OptionTab : public QWidget
{
public:
    explicit OptionTab(QWidget* parent);

    virtual ~OptionTab();

    virtual QString name()=0;
    virtual void apply(int *state);
    virtual void initValue(const QJsonObject& value);
    virtual QJsonObject toJson();
    virtual void notifyChanged(const QString& name,const QVariant& value);


};
}

#endif // OPTION_TAB_H
