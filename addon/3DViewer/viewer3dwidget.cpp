#include "viewer3dwidget.h"
#include "viewer3dconverter.h"

#include <QDebug>
#include <QMetaObject>
#include <QQmlContext>
#include <QQmlError>
#include <QQuickItem>
#include <QQuickWidget>
#include <QVBoxLayout>

Viewer3DWidget::Viewer3DWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("viewer3dWidget"));

    m_quickWidget = new QQuickWidget(this);
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->setClearColor(QColor(30, 34, 42));
    m_quickWidget->rootContext()->setContextProperty(QStringLiteral("viewer"), this);
    m_quickWidget->setSource(QUrl(QStringLiteral("qrc:/3dviewer/qml/viewer3d.qml")));

    if (m_quickWidget->status() == QQuickWidget::Error) {
        const QList<QQmlError> errors = m_quickWidget->errors();
        for (const QQmlError& error : errors) {
            qWarning() << "3DViewer qml:" << error.toString();
        }
        setOverlayMessage(tr("Qt Quick 3D scene can not be loaded."));
    } else {
        setOverlayMessage(tr("Select a model in the list to preview."));
    }

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_quickWidget);
    setLayout(layout);
}

Viewer3DWidget::~Viewer3DWidget()
{
}

bool Viewer3DWidget::active() const
{
    return m_active;
}

QUrl Viewer3DWidget::modelSource() const
{
    return m_modelSource;
}

bool Viewer3DWidget::wireframeMode() const
{
    return m_wireframeMode;
}

void Viewer3DWidget::setWireframe(bool on)
{
    if (m_wireframeMode == on) {
        return;
    }
    m_wireframeMode = on;
    emit wireframeModeChanged();
}

bool Viewer3DWidget::showGrid() const
{
    return m_showGrid;
}

void Viewer3DWidget::setShowGrid(bool on)
{
    if (m_showGrid == on) {
        return;
    }
    m_showGrid = on;
    emit showGridChanged();
}

QString Viewer3DWidget::overlayMessage() const
{
    return m_overlayMessage;
}

QString Viewer3DWidget::currentModelPath() const
{
    return m_modelPath;
}

QString Viewer3DWidget::boundsSummary() const
{
    return m_boundsSummary;
}

void Viewer3DWidget::loadModel(const QString& modelFilePath)
{
    QString error;
    const QString loadPath = Viewer3DConverter::resolveLoadPath(modelFilePath, &error);
    if (loadPath.isEmpty()) {
        m_modelPath = modelFilePath;
        setActive(false);
        setOverlayMessage(error);
        emit modelFailed(modelFilePath, error);
        return;
    }

    m_modelPath = modelFilePath;
    m_boundsSummary.clear();
    setActive(true);
    setOverlayMessage(QString());
    setModelSource(QUrl::fromLocalFile(loadPath));
}

void Viewer3DWidget::clear()
{
    m_modelPath.clear();
    m_boundsSummary.clear();
    setActive(false);
    setModelSource(QUrl());
    setOverlayMessage(tr("Select a model in the list to preview."));
}

void Viewer3DWidget::fitView()
{
    QQuickItem* root = m_quickWidget != nullptr ? m_quickWidget->rootObject() : nullptr;
    if (root == nullptr) {
        return;
    }
    QMetaObject::invokeMethod(root, "fitCameraToModel");
}

void Viewer3DWidget::notifyModelStatus(int status, const QString& errorString)
{
    if (status == 1) {
        setOverlayMessage(QString());
        emit modelReady(m_modelPath);
    } else if (status == 2) {
        const QString message = errorString.isEmpty() ? tr("Failed to load the model.") : errorString;
        setActive(false);
        setOverlayMessage(message);
        emit modelFailed(m_modelPath, message);
    }
}

void Viewer3DWidget::reportModelBounds(double dx, double dy, double dz, double radius, double distance)
{
    Q_UNUSED(distance);
    m_boundsSummary = tr("Size %1 x %2 x %3, radius %4")
                          .arg(dx, 0, 'f', 2)
                          .arg(dy, 0, 'f', 2)
                          .arg(dz, 0, 'f', 2)
                          .arg(radius, 0, 'f', 2);
    emit boundsChanged(m_boundsSummary);
}

void Viewer3DWidget::setActive(bool active)
{
    if (m_active == active) {
        return;
    }
    m_active = active;
    emit activeChanged();
}

void Viewer3DWidget::setModelSource(const QUrl& url)
{
    if (m_modelSource == url) {
        return;
    }
    m_modelSource = url;
    emit modelSourceChanged();
}

void Viewer3DWidget::setOverlayMessage(const QString& message)
{
    if (m_overlayMessage == message) {
        return;
    }
    m_overlayMessage = message;
    emit overlayMessageChanged();
}
