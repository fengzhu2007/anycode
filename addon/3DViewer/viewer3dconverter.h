#ifndef VIEWER3DCONVERTER_H
#define VIEWER3DCONVERTER_H

#include <QString>
#include <QVector>
#include <QVector3D>

/**
 * @brief 模型格式适配
 *
 * Qt Quick 3D 的 RuntimeLoader 原生支持 .gltf/.glb/.obj，.stl 与 .ply 需要先
 * 转换成 .obj 才能预览。转换结果写入模型同级目录的 ".cache"，
 * 文件名为 <模型名>_converted.obj，源文件更新后会自动重新转换。
 */
class Viewer3DConverter
{
public:
    struct MeshData {
        QVector<QVector3D> positions;
        QVector<QVector3D> normals;
        QVector<quint32> indices;
    };

    //返回可被 RuntimeLoader 直接读取的路径，失败时返回空字符串并写入 error
    static QString resolveLoadPath(const QString& modelFilePath, QString* error = nullptr);

    //把 .stl/.ply 转换为 .obj 缓存（已是最新时直接返回缓存路径）
    static QString ensureConverted(const QString& modelFilePath, QString* error = nullptr);

    //读取 .stl/.ply 文件内容为网格数据，供离线转换使用
    static bool readMesh(const QString& modelFilePath, MeshData* mesh, QString* error);

private:
    static bool readStl(const QString& filePath, MeshData* mesh, QString* error);
    static bool readPly(const QString& filePath, MeshData* mesh, QString* error);

    //合并重复顶点并计算平滑法线
    static void buildMesh(const QVector<QVector3D>& rawPositions,
                          const QVector<quint32>& rawIndices,
                          MeshData* mesh);
    static bool writeObj(const QString& targetPath, const QString& sourceName, const MeshData& mesh, QString* error);
};

#endif // VIEWER3DCONVERTER_H
