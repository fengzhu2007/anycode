#ifndef VIEWER3DTHUMBCACHE_H
#define VIEWER3DTHUMBCACHE_H

#include <QHash>
#include <QImage>
#include <QPixmap>
#include <QString>

/**
 * @brief 模型缩略图缓存
 *
 * 磁盘缓存固定落在模型同级目录的 ".cache" 中，文件名为 <模型名>_128x.png，
 * 与项目资源命名规则保持一致（如 CloseActive_16x.svg）。模型文件被修改后，
 * 由调用方通过 invalidate() 丢弃旧图并重新渲染。
 */
class Viewer3DThumbCache
{
public:
    Viewer3DThumbCache();

    //读取磁盘缓存，不存在或读取失败返回空图
    QPixmap load(const QString& modelFilePath) const;
    bool hasCache(const QString& modelFilePath) const;

    //保存离屏渲染帧，返回写入的图片路径；失败返回空字符串
    QString save(const QString& modelFilePath, const QImage& frame);

    //丢弃内存缓存（磁盘文件保持不变）
    void invalidate(const QString& modelFilePath);

    //删除单张缩略图与内存缓存
    void remove(const QString& modelFilePath);
    //清空某个目录下由本插件生成的缩略图
    void clearDirectory(const QString& dirPath);

    //占位图：等距线框立方体 + 格式角标
    QPixmap placeholder(const QString& modelFilePath, bool failed = false) const;

private:
    QPixmap decorate(const QPixmap& pixmap, const QString& modelFilePath, bool failed) const;

    mutable QHash<QString, QPixmap> m_memoryCache;
};

#endif // VIEWER3DTHUMBCACHE_H
