#ifndef VIEWER3DWIDGET_H
#define VIEWER3DWIDGET_H

#include <QWidget>
#include <QString>

namespace Qt3DCore {
class QEntity;
}

namespace Qt3DRender {
class QCamera;
class QMesh;
}

namespace Qt3DExtras {
class Qt3DWindow;
class QOrbitCameraController;
}

class Viewer3DWidget : public QWidget
{
    Q_OBJECT
public:
    explicit Viewer3DWidget(QWidget* parent = nullptr);
    ~Viewer3DWidget();

    void loadFile(const QString& filePath);
    void clearMesh();
    void resetView();

private:
    void setupScene();
    void setupGrid();
    void setupAxes();

    Qt3DExtras::Qt3DWindow* m_3dWindow;
    Qt3DCore::QEntity* m_rootEntity;
    Qt3DRender::QCamera* m_camera;
    Qt3DExtras::QOrbitCameraController* m_cameraController;
    Qt3DCore::QEntity* m_meshContainer;
};

#endif // VIEWER3DWIDGET_H
