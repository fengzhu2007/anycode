#include "viewer3dwidget.h"

#include <QVBoxLayout>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QNode>
#include <Qt3DRender/QMesh>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QRenderSettings>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DCore/QTransform>

static void appendVec3(QByteArray& data, const QVector3D& v)
{
    float coords[3] = { v.x(), v.y(), v.z() };
    data.append(reinterpret_cast<const char*>(coords), 3 * sizeof(float));
}

Viewer3DWidget::Viewer3DWidget(QWidget* parent)
    : QWidget(parent)
    , m_3dWindow(nullptr)
    , m_rootEntity(nullptr)
    , m_camera(nullptr)
    , m_cameraController(nullptr)
    , m_meshContainer(nullptr)
{
    setupScene();
}

Viewer3DWidget::~Viewer3DWidget()
{
}

void Viewer3DWidget::setupScene()
{
    m_3dWindow = new Qt3DExtras::Qt3DWindow();
    m_3dWindow->defaultFrameGraph()->setClearColor(QColor(46, 46, 50));

    // Root entity
    m_rootEntity = new Qt3DCore::QEntity();
    m_3dWindow->setRootEntity(m_rootEntity);

    // Camera
    m_camera = m_3dWindow->camera();
    m_camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    m_camera->setPosition(QVector3D(5, 5, 5));
    m_camera->setViewCenter(QVector3D(0, 0, 0));
    m_camera->setUpVector(QVector3D(0, 1, 0));

    // Orbit camera controller — 三维旋转
    m_cameraController = new Qt3DExtras::QOrbitCameraController(m_rootEntity);
    m_cameraController->setLinearSpeed(50.0f);
    m_cameraController->setLookSpeed(180.0f);
    m_cameraController->setCamera(m_camera);

    // Mesh container
    m_meshContainer = new Qt3DCore::QEntity(m_rootEntity);

    // Grid & axes
    setupGrid();
    setupAxes();

    // Embed Qt3DWindow into this widget
    auto* container = QWidget::createWindowContainer(m_3dWindow, this);
    container->setMinimumSize(400, 300);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(container);
    setLayout(layout);
}

void Viewer3DWidget::setupGrid()
{
    Qt3DCore::QEntity* gridEntity = new Qt3DCore::QEntity(m_rootEntity);

    Qt3DRender::QGeometry* geometry = new Qt3DRender::QGeometry(gridEntity);

    // Grid vertices (lines on XZ plane)
    QByteArray vertexData;
    int gridSize = 10;
    for(int i = -gridSize; i <= gridSize; ++i){
        QVector3D p1((float)i, 0.0f, (float)-gridSize);
        QVector3D p2((float)i, 0.0f, (float)gridSize);
        appendVec3(vertexData, p1);
        appendVec3(vertexData, p2);
        QVector3D p3((float)-gridSize, 0.0f, (float)i);
        QVector3D p4((float)gridSize, 0.0f, (float)i);
        appendVec3(vertexData, p3);
        appendVec3(vertexData, p4);
    }

    Qt3DRender::QBuffer* vertexBuffer = new Qt3DRender::QBuffer(geometry);
    vertexBuffer->setData(vertexData);

    Qt3DRender::QAttribute* posAttr = new Qt3DRender::QAttribute(geometry);
    posAttr->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
    posAttr->setVertexBaseType(Qt3DRender::QAttribute::Float);
    posAttr->setVertexSize(3);
    posAttr->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    posAttr->setBuffer(vertexBuffer);
    posAttr->setByteStride(3 * sizeof(float));
    posAttr->setCount(vertexData.size() / (3 * sizeof(float)));
    geometry->addAttribute(posAttr);

    Qt3DRender::QGeometryRenderer* renderer = new Qt3DRender::QGeometryRenderer(gridEntity);
    renderer->setGeometry(geometry);
    renderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);

    Qt3DCore::QTransform* transform = new Qt3DCore::QTransform(gridEntity);

    Qt3DExtras::QPhongMaterial* material = new Qt3DExtras::QPhongMaterial(gridEntity);
    material->setAmbient(QColor(70, 70, 80));
    material->setDiffuse(QColor(70, 70, 80));

    gridEntity->addComponent(renderer);
    gridEntity->addComponent(transform);
    gridEntity->addComponent(material);
}

void Viewer3DWidget::setupAxes()
{
    Qt3DCore::QEntity* axesEntity = new Qt3DCore::QEntity(m_rootEntity);

    Qt3DRender::QGeometry* geometry = new Qt3DRender::QGeometry(axesEntity);

    // 3 axis lines: 6 vertices
    QByteArray vertexData;
    float axisLen = 2.0f;
    QVector3D origin(0, 0, 0);
    QVector3D xAxis(axisLen, 0, 0);
    QVector3D yAxis(0, axisLen, 0);
    QVector3D zAxis(0, 0, axisLen);
    appendVec3(vertexData, origin);
    appendVec3(vertexData, xAxis);
    appendVec3(vertexData, origin);
    appendVec3(vertexData, yAxis);
    appendVec3(vertexData, origin);
    appendVec3(vertexData, zAxis);

    Qt3DRender::QBuffer* vertexBuffer = new Qt3DRender::QBuffer(geometry);
    vertexBuffer->setData(vertexData);

    Qt3DRender::QAttribute* posAttr = new Qt3DRender::QAttribute(geometry);
    posAttr->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
    posAttr->setVertexBaseType(Qt3DRender::QAttribute::Float);
    posAttr->setVertexSize(3);
    posAttr->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    posAttr->setBuffer(vertexBuffer);
    posAttr->setByteStride(3 * sizeof(float));
    posAttr->setCount(6);
    geometry->addAttribute(posAttr);

    Qt3DRender::QGeometryRenderer* renderer = new Qt3DRender::QGeometryRenderer(axesEntity);
    renderer->setGeometry(geometry);
    renderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);

    Qt3DCore::QTransform* transform = new Qt3DCore::QTransform(axesEntity);

    Qt3DExtras::QPhongMaterial* material = new Qt3DExtras::QPhongMaterial(axesEntity);
    material->setAmbient(QColor(200, 80, 80));
    material->setDiffuse(QColor(200, 80, 80));

    axesEntity->addComponent(renderer);
    axesEntity->addComponent(transform);
    axesEntity->addComponent(material);
}

void Viewer3DWidget::loadFile(const QString& filePath)
{
    clearMesh();

    Qt3DCore::QEntity* meshEntity = new Qt3DCore::QEntity(m_meshContainer);

    Qt3DRender::QMesh* mesh = new Qt3DRender::QMesh(meshEntity);
    mesh->setSource(QUrl::fromLocalFile(filePath));

    Qt3DExtras::QPhongMaterial* material = new Qt3DExtras::QPhongMaterial(meshEntity);
    material->setAmbient(QColor(80, 100, 180));
    material->setDiffuse(QColor(100, 140, 220));
    material->setSpecular(QColor(60, 60, 60));
    material->setShininess(32.0f);

    Qt3DCore::QTransform* transform = new Qt3DCore::QTransform(meshEntity);
    transform->setScale(1.0f);

    meshEntity->addComponent(mesh);
    meshEntity->addComponent(material);
    meshEntity->addComponent(transform);
}

void Viewer3DWidget::clearMesh()
{
    if(!m_meshContainer)
        return;
    const Qt3DCore::QNodeVector children = m_meshContainer->childNodes();
    for(Qt3DCore::QNode* child : children){
        Qt3DCore::QEntity* entity = qobject_cast<Qt3DCore::QEntity*>(child);
        if(entity)
            entity->setParent(static_cast<Qt3DCore::QNode*>(nullptr));
    }
}

void Viewer3DWidget::resetView()
{
    m_camera->setPosition(QVector3D(5, 5, 5));
    m_camera->setViewCenter(QVector3D(0, 0, 0));
}
