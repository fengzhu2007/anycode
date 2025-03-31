#ifndef SVG_VIEWER_H
#define SVG_VIEWER_H
#include "global.h"
#include <QScrollArea>
#include <QSvgWidget>
namespace ady{
class SVGViewPrivate;
class ANYENGINE_EXPORT SVGViewer : public QScrollArea
{
public:
    explicit SVGViewer(QWidget* parent=nullptr);
    ~SVGViewer();
    void setImagePath(const QString& path);
    QSize imageSize();
    QSize originalSize();
    void setZoom(float zoom);
    float zoom();
    QSvgRenderer* renderer();

protected:
    virtual void resizeEvent(QResizeEvent* e) override;


private:
    SVGViewPrivate* d;
};
}

#endif // SVG_VIEWER_H
