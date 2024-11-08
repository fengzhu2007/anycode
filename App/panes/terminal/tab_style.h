#ifndef TAB_STYLE_H
#define TAB_STYLE_H

#include <QProxyStyle>
namespace ady{
class TabStyle : public QProxyStyle
{
public:
    TabStyle() = default;
    QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const override;
    void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const override;
};
}
#endif // TAB_STYLE_H
