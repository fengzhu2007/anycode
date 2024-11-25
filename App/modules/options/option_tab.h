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
    virtual void apply();
    virtual void initValue(const QJsonObject& value);
    virtual QJsonObject toJson();


};
}

#endif // OPTION_TAB_H
