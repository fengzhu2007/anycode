#include "viewer3dconfig.h"

#include <QFileInfo>

namespace ady {
namespace Viewer3D {

static const int THUMBNAIL_SIZE = 128;

int thumbnailSize()
{
    return THUMBNAIL_SIZE;
}

QStringList nativeExtensions()
{
    return QStringList{"gltf", "glb", "obj"};
}

QStringList supportedExtensions()
{
    return QStringList{"gltf", "glb", "obj", "stl", "ply"};
}

bool isSupported(const QString& filePath)
{
    return supportedExtensions().contains(QFileInfo(filePath).suffix().toLower());
}

bool isNative(const QString& filePath)
{
    return nativeExtensions().contains(QFileInfo(filePath).suffix().toLower());
}

QString cacheDirectory(const QString& modelFilePath)
{
    return QFileInfo(modelFilePath).absolutePath() + QStringLiteral("/.cache");
}

QString thumbnailFilePath(const QString& modelFilePath)
{
    const QFileInfo fi(modelFilePath);
    return QStringLiteral("%1/%2_%3x.png")
        .arg(cacheDirectory(modelFilePath), fi.completeBaseName())
        .arg(THUMBNAIL_SIZE);
}

QString convertedFilePath(const QString& modelFilePath)
{
    const QFileInfo fi(modelFilePath);
    return QStringLiteral("%1/%2_converted.obj")
        .arg(cacheDirectory(modelFilePath), fi.completeBaseName());
}

}
}
