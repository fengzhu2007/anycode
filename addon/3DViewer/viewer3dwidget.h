#ifndef VIEWER3DWIDGET_H
#define VIEWER3DWIDGET_H

#include <QUrl>
#include <QWidget>

class QQuickWidget;

/**
 * @brief Qt Quick 3D 模型视图
 *
 * 内部用 QQuickWidget 承载 qml/viewer3d.qml，通过 Q_PROPERTY 把视图状态暴露给 QML，
 * 并通过 notifyModelStatus()/reportModelBounds() 接收 QML 的回调。
 * 相机适配(Fit-to-View)完全在 QML 中依据 RuntimeLoader.bounds 计算。
 */
class Viewer3DWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)
    Q_PROPERTY(QUrl modelSource READ modelSource NOTIFY modelSourceChanged)
    Q_PROPERTY(bool wireframeMode READ wireframeMode WRITE setWireframe NOTIFY wireframeModeChanged)
    Q_PROPERTY(bool showGrid READ showGrid WRITE setShowGrid NOTIFY showGridChanged)
    Q_PROPERTY(QString overlayMessage READ overlayMessage NOTIFY overlayMessageChanged)

public:
    explicit Viewer3DWidget(QWidget* parent = nullptr);
    ~Viewer3DWidget() override;

    bool active() const;
    QUrl modelSource() const;
    bool wireframeMode() const;
    void setWireframe(bool on);
    bool showGrid() const;
    void setShowGrid(bool on);
    QString overlayMessage() const;

    QString currentModelPath() const;
    QString boundsSummary() const;

    //载入模型（.stl/.ply 会自动转换后再载入）
    void loadModel(const QString& modelFilePath);
    void clear();
    void fitView();

    //供 viewer3d.qml 回调：1 加载成功，2 加载失败
    Q_INVOKABLE void notifyModelStatus(int status, const QString& errorString);
    //供 viewer3d.qml 回调模型包围盒信息
    Q_INVOKABLE void reportModelBounds(double dx, double dy, double dz, double radius, double distance);

signals:
    void activeChanged();
    void modelSourceChanged();
    void wireframeModeChanged();
    void showGridChanged();
    void overlayMessageChanged();

    void modelReady(const QString& modelFilePath);
    void modelFailed(const QString& modelFilePath, const QString& errorString);
    void boundsChanged(const QString& summary);

private:
    void setActive(bool active);
    void setModelSource(const QUrl& url);
    void setOverlayMessage(const QString& message);

    QQuickWidget* m_quickWidget = nullptr;
    QUrl m_modelSource;
    QString m_modelPath;
    QString m_overlayMessage;
    QString m_boundsSummary;
    bool m_active = false;
    bool m_wireframeMode = false;
    bool m_showGrid = true;
};

#endif // VIEWER3DWIDGET_H
