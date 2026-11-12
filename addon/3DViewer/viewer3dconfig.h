#ifndef VIEWER3DCONFIG_H
#define VIEWER3DCONFIG_H

#include <QString>
#include <QStringList>

namespace ady {
namespace Viewer3D {

// RuntimeLoader 可直接读取的格式：glTF 2.0(.gltf/.glb) 与 .obj
QStringList nativeExtensions();
// 浏览器中列出的全部模型格式
QStringList supportedExtensions();

bool isSupported(const QString& filePath);
bool isNative(const QString& filePath);

// 缩略图尺寸（缓存文件名中的 "_128x" 前缀即由此得到）
int thumbnailSize();

// 缓存目录固定为模型同级目录下的隐藏目录 ".cache"
QString cacheDirectory(const QString& modelFilePath);
// 缩略图：<模型名>_128x.png，沿用项目资源命名规则(如 CloseActive_16x.svg)
QString thumbnailFilePath(const QString& modelFilePath);
// 非原生格式(.stl/.ply)转换后的中间文件：<模型名>_converted.obj
QString convertedFilePath(const QString& modelFilePath);

}
}

#endif // VIEWER3DCONFIG_H
