#include "viewer3dthumbcache.h"
#include "viewer3dconfig.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>

Viewer3DThumbCache::Viewer3DThumbCache()
{
}

QPixmap Viewer3DThumbCache::load(const QString& modelFilePath) const
{
    const QString key = QDir::cleanPath(QFileInfo(modelFilePath).absoluteFilePath());
    auto iter = m_memoryCache.constFind(key);
    if (iter != m_memoryCache.constEnd()) {
        return iter.value();
    }

    const QString cachePath = ady::Viewer3D::thumbnailFilePath(modelFilePath);
    const QFileInfo cacheInfo(cachePath);
    const QFileInfo modelInfo(modelFilePath);
    if (!cacheInfo.exists() || cacheInfo.lastModified() < modelInfo.lastModified()) {
        return QPixmap();
    }

    QPixmap pixmap(cachePath);
    if (!pixmap.isNull()) {
        m_memoryCache.insert(key, pixmap);
    }
    return pixmap;
}

bool Viewer3DThumbCache::hasCache(const QString& modelFilePath) const
{
    return !load(modelFilePath).isNull();
}

QString Viewer3DThumbCache::save(const QString& modelFilePath, const QImage& frame)
{
    if (frame.isNull() || modelFilePath.isEmpty()) {
        return QString();
    }

    const int size = ady::Viewer3D::thumbnailSize();

    //居中裁剪为正方形再平滑缩放到缓存尺寸
    const int side = qMin(frame.width(), frame.height());
    const int x = (frame.width() - side) / 2;
    const int y = (frame.height() - side) / 2;
    const QImage square = frame.copy(x, y, side, side)
                              .scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QImage output(size, size, QImage::Format_ARGB32_Premultiplied);
    output.fill(Qt::transparent);

    QPainter painter(&output);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath clipPath;
    clipPath.addRoundedRect(QRectF(1, 1, size - 2, size - 2), 8, 8);
    painter.setClipPath(clipPath);
    painter.drawImage(0, 0, square);
    painter.setClipping(false);

    painter.setPen(QPen(QColor(90, 100, 120, 150), 1.0));
    painter.drawRoundedRect(QRectF(0.5, 0.5, size - 1, size - 1), 8, 8);
    painter.end();

    //缓存目录不存在时自动创建
    QDir dir(ady::Viewer3D::cacheDirectory(modelFilePath));
    if (!dir.exists() && !QDir().mkpath(dir.absolutePath())) {
        return QString();
    }

    const QString target = ady::Viewer3D::thumbnailFilePath(modelFilePath);
    if (!output.save(target, "PNG")) {
        return QString();
    }
    m_memoryCache.insert(QDir::cleanPath(QFileInfo(modelFilePath).absoluteFilePath()), QPixmap::fromImage(output));
    return target;
}

void Viewer3DThumbCache::invalidate(const QString& modelFilePath)
{
    m_memoryCache.remove(QDir::cleanPath(QFileInfo(modelFilePath).absoluteFilePath()));
}

void Viewer3DThumbCache::remove(const QString& modelFilePath)
{
    invalidate(modelFilePath);
    QFile::remove(ady::Viewer3D::thumbnailFilePath(modelFilePath));
}

void Viewer3DThumbCache::clearDirectory(const QString& dirPath)
{
    const QString cacheDir = QDir(dirPath).absoluteFilePath(QStringLiteral(".cache"));
    QDir dir(cacheDir);
    if (!dir.exists()) {
        return;
    }

    const QString suffix = QStringLiteral("_%1x.png").arg(ady::Viewer3D::thumbnailSize());
    const QFileInfoList files = dir.entryInfoList(QStringList() << "*.png", QDir::Files);
    for (const QFileInfo& fi : files) {
        if (fi.fileName().endsWith(suffix, Qt::CaseInsensitive)) {
            QFile::remove(fi.absoluteFilePath());
        }
    }
    m_memoryCache.clear();
}

QPixmap Viewer3DThumbCache::placeholder(const QString& modelFilePath, bool failed) const
{
    const int size = ady::Viewer3D::thumbnailSize();
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QLinearGradient gradient(0, 0, 0, size);
    gradient.setColorAt(0.0, QColor(44, 50, 62));
    gradient.setColorAt(1.0, QColor(30, 34, 42));

    const QRectF cardRect(1, 1, size - 2, size - 2);
    painter.setPen(QPen(QColor(74, 84, 104), 1.0));
    painter.setBrush(gradient);
    painter.drawRoundedRect(cardRect, 8, 8);

    //等距投影线框立方体
    const QPointF center(size * 0.5, size * 0.45);
    const qreal radius = size * 0.22;
    const QPointF top(center.x(), center.y() - radius);
    const QPointF topLeft(center.x() - radius * 0.86, center.y() - radius * 0.5);
    const QPointF topRight(center.x() + radius * 0.86, center.y() - radius * 0.5);
    const QPointF bottom(center.x(), center.y() + radius);
    const QPointF bottomLeft(center.x() - radius * 0.86, center.y() + radius * 0.5);
    const QPointF bottomRight(center.x() + radius * 0.86, center.y() + radius * 0.5);

    painter.setPen(QPen(failed ? QColor(224, 108, 117, 210) : QColor(97, 175, 239, 210),
                        1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    QPolygonF hexagon;
    hexagon << top << topRight << bottomRight << bottom << bottomLeft << topLeft;
    painter.drawPolygon(hexagon);
    painter.drawLine(center, topLeft);
    painter.drawLine(center, topRight);
    painter.drawLine(center, bottom);
    painter.end();

    return decorate(pixmap, modelFilePath, failed);
}

QPixmap Viewer3DThumbCache::decorate(const QPixmap& pixmap, const QString& modelFilePath, bool failed) const
{
    QPixmap result = pixmap;
    const int size = ady::Viewer3D::thumbnailSize();

    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QString suffix = QFileInfo(modelFilePath).suffix().toUpper();
    if (!suffix.isEmpty()) {
        QFont font = painter.font();
        font.setPixelSize(10);
        font.setBold(true);
        painter.setFont(font);

        const QRectF badgeRect(6, 6, 40, 16);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(18, 22, 28, 200));
        painter.drawRoundedRect(badgeRect, 4, 4);
        painter.setPen(failed ? QColor(224, 108, 117) : QColor(152, 195, 121));
        painter.drawText(badgeRect, Qt::AlignCenter, suffix);
    }

    if (failed) {
        QFont font = painter.font();
        font.setPixelSize(10);
        font.setBold(false);
        painter.setFont(font);
        const QRectF statusRect(6, size - 22, size - 12, 16);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(18, 22, 28, 210));
        painter.drawRoundedRect(statusRect, 4, 4);
        painter.setPen(QColor(224, 108, 117));
        painter.drawText(statusRect, Qt::AlignCenter, QStringLiteral("load failed"));
    }
    painter.end();
    return result;
}
