#ifndef VIEWER3DTHUMBNAILRENDERER_H
#define VIEWER3DTHUMBNAILRENDERER_H

#include <QObject>
#include <QQueue>
#include <QSet>
#include <QString>
#include <QStringList>

class QQuickItem;
class QQuickView;
class QTimer;
class Viewer3DThumbCache;

/**
 * @brief 缩略图批量渲染队列
 *
 * 使用一个离屏(屏幕外定位)的 QQuickView 承载 Qt Quick 3D 场景，逐个载入模型、
 * 自动适配相机后抓取渲染帧，交由 Viewer3DThumbCache 写入模型同级目录的 ".cache"。
 * 队列支持可见区域优先(prioritize)，已存在缓存的模型会被自动跳过。
 */
class Viewer3DThumbnailRenderer : public QObject
{
    Q_OBJECT
public:
    explicit Viewer3DThumbnailRenderer(Viewer3DThumbCache* cache, QObject* parent = nullptr);
    ~Viewer3DThumbnailRenderer() override;

    //加入渲染队列，highPriority 时插到队首
    void enqueue(const QString& modelFilePath, bool highPriority = false);
    //可见区域内的模型优先渲染
    void prioritize(const QStringList& modelFilePaths);
    void clearQueue();
    int pendingCount() const;

    //供 thumbnailrenderer3d.qml 回调模型加载结果：1 成功，2 失败
    Q_INVOKABLE void notifyModelStatus(int status, const QString& errorString);

signals:
    void thumbnailGenerated(const QString& modelFilePath, const QString& thumbnailPath);
    void thumbnailFailed(const QString& modelFilePath, const QString& errorString);
    void queueProgressChanged(int remainingTasks);

private:
    void ensureView();
    void releaseView();
    void processNext();
    void startTask(const QString& modelFilePath);
    void grabAndSave();
    void finishCurrent(const QString& errorString);

    Viewer3DThumbCache* m_cache = nullptr;
    QQuickView* m_view = nullptr;
    QQuickItem* m_rootItem = nullptr;
    QTimer* m_watchdog = nullptr;

    QQueue<QString> m_queue;
    QSet<QString> m_queued;
    QString m_current;
    bool m_busy = false;
    bool m_grabPending = false;
};

#endif // VIEWER3DTHUMBNAILRENDERER_H
