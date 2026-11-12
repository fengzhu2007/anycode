#include "viewer3dthumbnailrenderer.h"
#include "viewer3dconverter.h"
#include "viewer3dthumbcache.h"

#include <QQuickItem>
#include <QQuickView>
#include <QQmlContext>
#include <QQmlError>
#include <QTimer>
#include <QUrl>
#include <QDebug>

static const int WATCHDOG_TIMEOUT = 15000;
static const int SETTLE_DELAY = 220;

Viewer3DThumbnailRenderer::Viewer3DThumbnailRenderer(Viewer3DThumbCache* cache, QObject* parent)
    : QObject(parent)
    , m_cache(cache)
{
    m_watchdog = new QTimer(this);
    m_watchdog->setSingleShot(true);
    connect(m_watchdog, &QTimer::timeout, this, [this]() {
        finishCurrent(tr("Rendering timed out."));
    });
}

Viewer3DThumbnailRenderer::~Viewer3DThumbnailRenderer()
{
    if (m_view) {
        m_view->hide();
        delete m_view;
        m_view = nullptr;
    }
}

int Viewer3DThumbnailRenderer::pendingCount() const
{
    return m_queue.size() + (m_busy ? 1 : 0);
}

void Viewer3DThumbnailRenderer::enqueue(const QString& modelFilePath, bool highPriority)
{
    if (modelFilePath.isEmpty() || m_cache == nullptr) {
        return;
    }
    if (m_queued.contains(modelFilePath) || m_current == modelFilePath) {
        if (highPriority && m_queue.removeOne(modelFilePath)) {
            m_queue.prepend(modelFilePath);
        }
        return;
    }
    if (m_cache->hasCache(modelFilePath)) {
        return;
    }

    m_queued.insert(modelFilePath);
    if (highPriority) {
        m_queue.prepend(modelFilePath);
    } else {
        m_queue.enqueue(modelFilePath);
    }
    emit queueProgressChanged(pendingCount());

    if (!m_busy) {
        QTimer::singleShot(0, this, &Viewer3DThumbnailRenderer::processNext);
    }
}

void Viewer3DThumbnailRenderer::prioritize(const QStringList& modelFilePaths)
{
    if (modelFilePaths.isEmpty()) {
        return;
    }
    for (int i = modelFilePaths.size() - 1; i >= 0; --i) {
        const QString filePath = modelFilePaths.at(i);
        if (m_queue.removeOne(filePath)) {
            m_queue.prepend(filePath);
        }
    }
    if (!m_busy) {
        QTimer::singleShot(0, this, &Viewer3DThumbnailRenderer::processNext);
    }
}

void Viewer3DThumbnailRenderer::clearQueue()
{
    m_queue.clear();
    m_queued.clear();
    emit queueProgressChanged(pendingCount());
}

void Viewer3DThumbnailRenderer::ensureView()
{
    if (m_view != nullptr) {
        if (!m_view->isVisible()) {
            m_view->show();
        }
        return;
    }

    //离屏窗口只用于抓帧，不参与用户交互
    m_view = new QQuickView();
    m_view->setFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
    m_view->setColor(QColor(30, 34, 42));
    m_view->setResizeMode(QQuickView::SizeRootObjectToView);
    m_view->resize(256, 256);
    m_view->rootContext()->setContextProperty(QStringLiteral("thumbGenerator"), this);
    m_view->setSource(QUrl(QStringLiteral("qrc:/3dviewer/qml/thumbnailrenderer3d.qml")));
    m_rootItem = m_view->rootObject();
    if (m_view->status() == QQuickView::Error || m_rootItem == nullptr) {
        const QList<QQmlError> errors = m_view->errors();
        for (const QQmlError& error : errors) {
            qWarning() << "3DViewer thumbnail qml:" << error.toString();
        }
        delete m_view;
        m_view = nullptr;
        m_rootItem = nullptr;
        return;
    }

    //放到屏幕之外，避免遮挡用户界面
    m_view->setPosition(-32000, -32000);
    m_view->show();
}

void Viewer3DThumbnailRenderer::releaseView()
{
    if (m_view != nullptr && !m_busy && m_queue.isEmpty()) {
        m_view->hide();
    }
}

void Viewer3DThumbnailRenderer::processNext()
{
    if (m_busy) {
        return;
    }

    while (!m_queue.isEmpty()) {
        const QString modelFilePath = m_queue.dequeue();
        m_queued.remove(modelFilePath);

        if (m_cache->hasCache(modelFilePath)) {
            continue;
        }
        startTask(modelFilePath);
        return;
    }

    emit queueProgressChanged(0);
    releaseView();
}

void Viewer3DThumbnailRenderer::startTask(const QString& modelFilePath)
{
    QString error;
    const QString loadPath = Viewer3DConverter::resolveLoadPath(modelFilePath, &error);
    if (loadPath.isEmpty()) {
        emit thumbnailFailed(modelFilePath, error);
        QTimer::singleShot(0, this, &Viewer3DThumbnailRenderer::processNext);
        return;
    }

    ensureView();
    if (m_rootItem == nullptr) {
        emit thumbnailFailed(modelFilePath, tr("Qt Quick 3D is not available."));
        QTimer::singleShot(0, this, &Viewer3DThumbnailRenderer::processNext);
        return;
    }

    m_busy = true;
    m_grabPending = false;
    m_current = modelFilePath;

    //先清空再设置，确保重复载入同一路径时也能触发 RuntimeLoader 重新加载
    m_rootItem->setProperty("modelSource", QUrl());
    m_rootItem->setProperty("modelSource", QUrl::fromLocalFile(loadPath));

    m_watchdog->start(WATCHDOG_TIMEOUT);
    emit queueProgressChanged(pendingCount());
}

void Viewer3DThumbnailRenderer::notifyModelStatus(int status, const QString& errorString)
{
    if (!m_busy || m_current.isEmpty()) {
        return;
    }

    if (status == 1) {
        //等待若干帧渲染完成后再抓图
        QTimer::singleShot(SETTLE_DELAY, this, &Viewer3DThumbnailRenderer::grabAndSave);
    } else if (status == 2) {
        finishCurrent(errorString.isEmpty() ? tr("Failed to load the model.") : errorString);
    }
}

void Viewer3DThumbnailRenderer::grabAndSave()
{
    if (!m_busy || m_view == nullptr) {
        return;
    }

    const QImage frame = m_view->grabWindow();
    if (frame.isNull() || frame.width() < 8) {
        if (!m_grabPending) {
            //部分驱动首帧抓取为空，重试一次
            m_grabPending = true;
            QTimer::singleShot(SETTLE_DELAY, this, &Viewer3DThumbnailRenderer::grabAndSave);
            return;
        }
        finishCurrent(tr("Can not grab the rendered frame."));
        return;
    }

    const QString savedPath = m_cache->save(m_current, frame);
    if (savedPath.isEmpty()) {
        finishCurrent(tr("Can not write the thumbnail cache file."));
        return;
    }

    const QString currentFile = m_current;
    m_current.clear();
    m_busy = false;
    m_grabPending = false;
    m_watchdog->stop();

    emit thumbnailGenerated(currentFile, savedPath);
    emit queueProgressChanged(pendingCount());
    QTimer::singleShot(0, this, &Viewer3DThumbnailRenderer::processNext);
}

void Viewer3DThumbnailRenderer::finishCurrent(const QString& errorString)
{
    if (!m_busy) {
        return;
    }

    const QString currentFile = m_current;
    m_current.clear();
    m_busy = false;
    m_grabPending = false;
    m_watchdog->stop();

    if (!currentFile.isEmpty()) {
        emit thumbnailFailed(currentFile, errorString);
    }
    emit queueProgressChanged(pendingCount());
    QTimer::singleShot(0, this, &Viewer3DThumbnailRenderer::processNext);
}
