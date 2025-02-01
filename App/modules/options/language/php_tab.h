#ifndef PHP_TAB_H
#define PHP_TAB_H
#include "../option_tab.h"

namespace Ui {
class PHPTab;
}
namespace ady{
class PHPTab : public OptionTab
{
    Q_OBJECT
public:
    explicit PHPTab(QWidget *parent = nullptr);
    ~PHPTab();

    virtual QString name() override;
    virtual void apply(int *state) override;
    virtual void initValue(const QJsonObject& value) override;
    virtual QJsonObject toJson() override;
    virtual void notifyChanged(const QString& name,const QVariant& value) override;

    void initView();

public slots:
    void onStateChanged(int state);

private:
    Ui::PHPTab *ui;
};
}
#endif // PHP_TAB_H
