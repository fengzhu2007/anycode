#include "viewer3dconverter.h"
#include "viewer3dconfig.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QObject>
#include <QRegularExpression>

#include <cstring>

namespace {

constexpr double kAxisValueScale = 10000.0;

//顶点合并用的近似哈希
quint64 positionKey(const QVector3D& v)
{
    const qint64 x = qRound64(v.x() * kAxisValueScale);
    const qint64 y = qRound64(v.y() * kAxisValueScale);
    const qint64 z = qRound64(v.z() * kAxisValueScale);
    return quint64((x * 73856093) ^ (y * 19349663) ^ (z * 83492791));
}

//STL 采用 Z 轴向上的建模习惯，输出 OBJ 时旋转为 Y 轴向上
inline QVector3D stlToYUp(const QVector3D& v)
{
    return QVector3D(v.x(), v.z(), -v.y());
}

void appendNumber(QByteArray& buffer, double value)
{
    buffer.append(QByteArray::number(value, 'f', 6));
}

}

QString Viewer3DConverter::resolveLoadPath(const QString& modelFilePath, QString* error)
{
    if (ady::Viewer3D::isNative(modelFilePath)) {
        return modelFilePath;
    }
    return ensureConverted(modelFilePath, error);
}

QString Viewer3DConverter::ensureConverted(const QString& modelFilePath, QString* error)
{
    const QFileInfo sourceInfo(modelFilePath);
    if (!sourceInfo.exists()) {
        if (error) {
            *error = QObject::tr("File does not exist: %1").arg(modelFilePath);
        }
        return QString();
    }

    const QString targetPath = ady::Viewer3D::convertedFilePath(modelFilePath);
    const QFileInfo targetInfo(targetPath);
    if (targetInfo.exists() && targetInfo.lastModified() >= sourceInfo.lastModified()) {
        return targetPath;
    }

    const QString suffix = sourceInfo.suffix().toLower();
    if (suffix != QLatin1String("stl") && suffix != QLatin1String("ply")) {
        if (error) {
            *error = QObject::tr("Unsupported model format: .%1").arg(suffix);
        }
        return QString();
    }

    MeshData mesh;
    if (!readMesh(modelFilePath, &mesh, error)) {
        return QString();
    }

    QDir dir(ady::Viewer3D::cacheDirectory(modelFilePath));
    if (!dir.exists() && !QDir().mkpath(dir.absolutePath())) {
        if (error) {
            *error = QObject::tr("Can not create cache directory: %1").arg(dir.absolutePath());
        }
        return QString();
    }

    if (!writeObj(targetPath, sourceInfo.completeBaseName(), mesh, error)) {
        return QString();
    }
    return targetPath;
}

bool Viewer3DConverter::readMesh(const QString& modelFilePath, MeshData* mesh, QString* error)
{
    const QString suffix = QFileInfo(modelFilePath).suffix().toLower();
    if (suffix == QLatin1String("stl")) {
        return readStl(modelFilePath, mesh, error);
    } else if (suffix == QLatin1String("ply")) {
        return readPly(modelFilePath, mesh, error);
    }
    if (error) {
        *error = QObject::tr("Unsupported model format: .%1").arg(suffix);
    }
    return false;
}

bool Viewer3DConverter::readStl(const QString& filePath, MeshData* mesh, QString* error)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) {
            *error = QObject::tr("Can not open file: %1").arg(filePath);
        }
        return false;
    }
    const QByteArray data = file.readAll();
    file.close();

    QVector<QVector3D> rawPositions;

    //二进制 STL：80 字节文件头 + 4 字节三角形数量 + 每个三角形 50 字节
    bool binary = false;
    if (data.size() >= 84) {
        const quint32 triangles = quint32(uchar(data.at(80)))
            | (quint32(uchar(data.at(81))) << 8)
            | (quint32(uchar(data.at(82))) << 16)
            | (quint32(uchar(data.at(83))) << 24);
        binary = (qint64(84) + qint64(triangles) * 50 == data.size());
    }

    if (binary) {
        const quint32 triangles = quint32(uchar(data.at(80)))
            | (quint32(uchar(data.at(81))) << 8)
            | (quint32(uchar(data.at(82))) << 16)
            | (quint32(uchar(data.at(83))) << 24);
        rawPositions.reserve(int(triangles) * 3);

        const char* cursor = data.constData() + 84;
        for (quint32 i = 0; i < triangles; ++i) {
            //跳过法线，只取三个顶点
            float values[12];
            memcpy(values, cursor, sizeof(values));
            for (int j = 0; j < 3; ++j) {
                const float* v = values + 3 + j * 3;
                rawPositions.append(stlToYUp(QVector3D(v[0], v[1], v[2])));
            }
            cursor += 50;
        }
    } else {
        //ASCII STL：顺序提取所有 vertex 行
        const QRegularExpression regex(QStringLiteral("vertex\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)"));
        auto iter = regex.globalMatch(QString::fromLatin1(data));
        while (iter.hasNext()) {
            const auto match = iter.next();
            bool okX = false, okY = false, okZ = false;
            const float x = match.capturedView(1).toFloat(&okX);
            const float y = match.capturedView(2).toFloat(&okY);
            const float z = match.capturedView(3).toFloat(&okZ);
            if (okX && okY && okZ) {
                rawPositions.append(stlToYUp(QVector3D(x, y, z)));
            }
        }
    }

    if (rawPositions.isEmpty()) {
        if (error) {
            *error = QObject::tr("No triangle found in the STL file.");
        }
        return false;
    }

    QVector<quint32> rawIndices;
    rawIndices.resize(rawPositions.size());
    for (int i = 0; i < rawIndices.size(); ++i) {
        rawIndices[i] = quint32(i);
    }
    buildMesh(rawPositions, rawIndices, mesh);
    return true;
}

namespace {

enum PlyType {
    PlyInt8 = 0,
    PlyUInt8,
    PlyInt16,
    PlyUInt16,
    PlyInt32,
    PlyUInt32,
    PlyFloat32,
    PlyFloat64,
    PlyUnknown
};

PlyType plyTypeFromName(const QByteArray& name)
{
    const QByteArray lower = name.toLower();
    if (lower == "char" || lower == "int8") return PlyInt8;
    if (lower == "uchar" || lower == "uint8") return PlyUInt8;
    if (lower == "short" || lower == "int16") return PlyInt16;
    if (lower == "ushort" || lower == "uint16") return PlyUInt16;
    if (lower == "int" || lower == "int32") return PlyInt32;
    if (lower == "uint" || lower == "uint32") return PlyUInt32;
    if (lower == "float" || lower == "float32") return PlyFloat32;
    if (lower == "double" || lower == "float64") return PlyFloat64;
    return PlyUnknown;
}

int plyTypeSize(PlyType type)
{
    switch (type) {
    case PlyInt8:
    case PlyUInt8:
        return 1;
    case PlyInt16:
    case PlyUInt16:
        return 2;
    case PlyInt32:
    case PlyUInt32:
    case PlyFloat32:
        return 4;
    case PlyFloat64:
        return 8;
    default:
        return 0;
    }
}

struct PlyProperty {
    QByteArray name;
    PlyType type = PlyUnknown;
    bool isList = false;
    PlyType countType = PlyUnknown;
};

struct PlyElement {
    QByteArray name;
    int count = 0;
    QVector<PlyProperty> properties;
};

//顺序读取 PLY 主体数据，同时兼容 ascii 与 binary_little_endian
class PlyStream
{
public:
    PlyStream(const QByteArray& body, bool ascii, bool bigEndian)
        : m_body(body), m_ascii(ascii), m_bigEndian(bigEndian)
    {
    }

    bool read(PlyType type, double* value)
    {
        if (m_ascii) {
            const QByteArray token = nextToken();
            if (token.isEmpty()) {
                return false;
            }
            bool ok = false;
            *value = token.toDouble(&ok);
            return ok;
        }

        const int size = plyTypeSize(type);
        if (size <= 0 || m_offset + size > m_body.size()) {
            return false;
        }
        const uchar* raw = reinterpret_cast<const uchar*>(m_body.constData() + m_offset);
        m_offset += size;

        auto readInteger = [&](int bytes) -> qint64 {
            qint64 result = 0;
            for (int i = 0; i < bytes; ++i) {
                const int index = m_bigEndian ? i : bytes - 1 - i;
                result = (result << 8) | qint64(raw[index]);
            }
            return result;
        };

        switch (type) {
        case PlyInt8:
            *value = qint8(readInteger(1));
            break;
        case PlyUInt8:
            *value = quint8(readInteger(1));
            break;
        case PlyInt16:
            *value = qint16(readInteger(2));
            break;
        case PlyUInt16:
            *value = quint16(readInteger(2));
            break;
        case PlyInt32:
            *value = qint32(readInteger(4));
            break;
        case PlyUInt32:
            *value = qint64(quint32(readInteger(4)));
            break;
        case PlyFloat32: {
            quint32 bits = quint32(readInteger(4));
            float number = 0.0f;
            memcpy(&number, &bits, sizeof(float));
            *value = number;
            break;
        }
        case PlyFloat64: {
            quint64 bits = quint64(readInteger(8));
            double number = 0.0;
            memcpy(&number, &bits, sizeof(double));
            *value = number;
            break;
        }
        default:
            return false;
        }
        return true;
    }

private:
    QByteArray nextToken()
    {
        while (m_offset < m_body.size() && isSpace(m_body.at(m_offset))) {
            ++m_offset;
        }
        const int start = m_offset;
        while (m_offset < m_body.size() && !isSpace(m_body.at(m_offset))) {
            ++m_offset;
        }
        return m_body.mid(start, m_offset - start);
    }

    static bool isSpace(char c)
    {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    }

    QByteArray m_body;
    bool m_ascii = true;
    bool m_bigEndian = false;
    int m_offset = 0;
};

}

bool Viewer3DConverter::readPly(const QString& filePath, MeshData* mesh, QString* error)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) {
            *error = QObject::tr("Can not open file: %1").arg(filePath);
        }
        return false;
    }
    const QByteArray data = file.readAll();
    file.close();

    //解析头部
    const int headerEnd = data.indexOf("end_header");
    if (headerEnd < 0) {
        if (error) {
            *error = QObject::tr("Invalid PLY header.");
        }
        return false;
    }
    int bodyOffset = headerEnd + int(qstrlen("end_header"));
    while (bodyOffset < data.size() && (data.at(bodyOffset) == '\r' || data.at(bodyOffset) == '\n')) {
        ++bodyOffset;
    }

    bool ascii = true;
    bool bigEndian = false;
    QVector<PlyElement> elements;

    const QList<QByteArray> lines = data.left(headerEnd).split('\n');
    for (const QByteArray& rawLine : lines) {
        const QByteArray line = rawLine.trimmed();
        if (line.isEmpty()) {
            continue;
        }
        const QList<QByteArray> tokens = line.simplified().split(' ');
        if (tokens.isEmpty()) {
            continue;
        }
        if (tokens.first() == "format") {
            if (tokens.size() < 2) {
                continue;
            }
            if (tokens.at(1) == "ascii") {
                ascii = true;
            } else if (tokens.at(1) == "binary_little_endian") {
                ascii = false;
                bigEndian = false;
            } else if (tokens.at(1) == "binary_big_endian") {
                ascii = false;
                bigEndian = true;
            }
        } else if (tokens.first() == "element") {
            if (tokens.size() < 3) {
                continue;
            }
            PlyElement element;
            element.name = tokens.at(1);
            element.count = tokens.at(2).toInt();
            elements.append(element);
        } else if (tokens.first() == "property" && !elements.isEmpty()) {
            PlyProperty property;
            if (tokens.size() >= 5 && tokens.at(1) == "list") {
                property.isList = true;
                property.countType = plyTypeFromName(tokens.at(2));
                property.type = plyTypeFromName(tokens.at(3));
                property.name = tokens.at(4);
            } else if (tokens.size() >= 3) {
                property.type = plyTypeFromName(tokens.at(1));
                property.name = tokens.at(2);
            }
            elements.last().properties.append(property);
        }
    }

    PlyStream stream(data.mid(bodyOffset), ascii, bigEndian);

    QVector<QVector3D> rawPositions;
    QVector<quint32> rawIndices;

    for (const PlyElement& element : elements) {
        const bool isVertex = element.name == "vertex";
        const bool isFace = element.name == "face";

        for (int i = 0; i < element.count; ++i) {
            double x = 0.0, y = 0.0, z = 0.0;
            bool hasX = false, hasY = false, hasZ = false;
            QVector<quint32> faceIndices;

            for (const PlyProperty& property : element.properties) {
                if (property.isList) {
                    double count = 0.0;
                    if (!stream.read(property.countType, &count)) {
                        if (error) {
                            *error = QObject::tr("Unexpected end of PLY data.");
                        }
                        return false;
                    }
                    const int listCount = int(count);
                    for (int k = 0; k < listCount; ++k) {
                        double value = 0.0;
                        if (!stream.read(property.type, &value)) {
                            if (error) {
                                *error = QObject::tr("Unexpected end of PLY data.");
                            }
                            return false;
                        }
                        if (isFace && property.name.startsWith("vertex_indices")) {
                            faceIndices.append(quint32(qMax(0.0, value)));
                        }
                    }
                } else {
                    double value = 0.0;
                    if (!stream.read(property.type, &value)) {
                        if (error) {
                            *error = QObject::tr("Unexpected end of PLY data.");
                        }
                        return false;
                    }
                    if (isVertex) {
                        if (property.name == "x") {
                            x = value;
                            hasX = true;
                        } else if (property.name == "y") {
                            y = value;
                            hasY = true;
                        } else if (property.name == "z") {
                            z = value;
                            hasZ = true;
                        }
                    }
                }
            }

            if (isVertex && hasX && hasY && hasZ) {
                rawPositions.append(QVector3D(float(x), float(y), float(z)));
            }

            //扇形三角化面片
            if (isFace && faceIndices.size() >= 3) {
                for (int k = 2; k < faceIndices.size(); ++k) {
                    rawIndices.append(faceIndices.at(0));
                    rawIndices.append(faceIndices.at(k - 1));
                    rawIndices.append(faceIndices.at(k));
                }
            }
        }
    }

    if (rawPositions.isEmpty() || rawIndices.isEmpty()) {
        if (error) {
            *error = QObject::tr("No triangle found in the PLY file.");
        }
        return false;
    }
    buildMesh(rawPositions, rawIndices, mesh);
    return true;
}

void Viewer3DConverter::buildMesh(const QVector<QVector3D>& rawPositions,
                                  const QVector<quint32>& rawIndices,
                                  MeshData* mesh)
{
    mesh->positions.clear();
    mesh->normals.clear();
    mesh->indices.clear();

    QHash<quint64, QVector<quint32>> buckets;
    QVector<quint32> remap;
    remap.resize(rawPositions.size());

    const double mergeTolerance = 1.0 / kAxisValueScale;
    for (int i = 0; i < rawPositions.size(); ++i) {
        const QVector3D& position = rawPositions.at(i);
        const quint64 key = positionKey(position);
        auto iter = buckets.find(key);
        if (iter != buckets.end()) {
            bool merged = false;
            for (quint32 candidate : iter.value()) {
                if ((mesh->positions.at(int(candidate)) - position).length() <= mergeTolerance) {
                    remap[i] = candidate;
                    merged = true;
                    break;
                }
            }
            if (merged) {
                continue;
            }
        }
        const quint32 index = quint32(mesh->positions.size());
        mesh->positions.append(position);
        buckets[key].append(index);
        remap[i] = index;
    }

    QVector<QVector3D> normalSum;
    normalSum.resize(mesh->positions.size());

    for (int i = 0; i + 2 < rawIndices.size(); i += 3) {
        const quint32 i0 = remap.at(int(rawIndices.at(i)));
        const quint32 i1 = remap.at(int(rawIndices.at(i + 1)));
        const quint32 i2 = remap.at(int(rawIndices.at(i + 2)));

        const QVector3D& p0 = mesh->positions.at(int(i0));
        const QVector3D& p1 = mesh->positions.at(int(i1));
        const QVector3D& p2 = mesh->positions.at(int(i2));

        QVector3D normal = QVector3D::crossProduct(p1 - p0, p2 - p0);
        if (normal.lengthSquared() <= 0.0f) {
            continue;
        }
        normalSum[int(i0)] += normal;
        normalSum[int(i1)] += normal;
        normalSum[int(i2)] += normal;

        mesh->indices.append(i0);
        mesh->indices.append(i1);
        mesh->indices.append(i2);
    }

    mesh->normals.resize(mesh->positions.size());
    for (int i = 0; i < normalSum.size(); ++i) {
        const QVector3D normal = normalSum.at(i);
        if (normal.lengthSquared() > 0.0f) {
            mesh->normals[i] = normal.normalized();
        } else {
            mesh->normals[i] = QVector3D(0.0f, 1.0f, 0.0f);
        }
    }
}

bool Viewer3DConverter::writeObj(const QString& targetPath, const QString& sourceName, const MeshData& mesh, QString* error)
{
    QByteArray buffer;
    const int estimate = mesh.positions.size() * 40 + mesh.indices.size() * 12 + 128;
    buffer.reserve(estimate);

    buffer.append("# Converted by anycode 3D Viewer from ");
    buffer.append(sourceName.toUtf8());
    buffer.append("\no ");
    buffer.append(sourceName.toUtf8());
    buffer.append("\n");

    for (const QVector3D& position : mesh.positions) {
        buffer.append("v ");
        appendNumber(buffer, position.x());
        buffer.append(' ');
        appendNumber(buffer, position.y());
        buffer.append(' ');
        appendNumber(buffer, position.z());
        buffer.append('\n');
    }

    for (const QVector3D& normal : mesh.normals) {
        buffer.append("vn ");
        appendNumber(buffer, normal.x());
        buffer.append(' ');
        appendNumber(buffer, normal.y());
        buffer.append(' ');
        appendNumber(buffer, normal.z());
        buffer.append('\n');
    }

    for (int i = 0; i + 2 < mesh.indices.size(); i += 3) {
        const quint32 i0 = mesh.indices.at(i) + 1;
        const quint32 i1 = mesh.indices.at(i + 1) + 1;
        const quint32 i2 = mesh.indices.at(i + 2) + 1;
        buffer.append("f ");
        buffer.append(QByteArray::number(i0)).append("//").append(QByteArray::number(i0)).append(' ');
        buffer.append(QByteArray::number(i1)).append("//").append(QByteArray::number(i1)).append(' ');
        buffer.append(QByteArray::number(i2)).append("//").append(QByteArray::number(i2)).append('\n');
    }

    QFile file(targetPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error) {
            *error = QObject::tr("Can not write cache file: %1").arg(targetPath);
        }
        return false;
    }
    const qint64 written = file.write(buffer);
    file.close();
    if (written != buffer.size()) {
        if (error) {
            *error = QObject::tr("Can not write cache file: %1").arg(targetPath);
        }
        return false;
    }
    return true;
}
