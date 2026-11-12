import QtQuick
import QtQuick3D
import QtQuick3D.AssetUtils

/**
 * 缩略图离屏渲染场景：不加载任何交互控件，
 * 载入模型后依据包围球自动适配相机并回报状态给 Viewer3DThumbnailRenderer。
 */
Item {
    id: offscreenRoot
    width: 256
    height: 256

    property url modelSource: ""

    function fitCameraToModel() {
        if (loader.status !== RuntimeLoader.Success)
            return

        var minB = loader.bounds.minimum
        var maxB = loader.bounds.maximum

        var cx = (minB.x + maxB.x) * 0.5
        var cy = (minB.y + maxB.y) * 0.5
        var cz = (minB.z + maxB.z) * 0.5

        var dx = Math.abs(maxB.x - minB.x)
        var dy = Math.abs(maxB.y - minB.y)
        var dz = Math.abs(maxB.z - minB.z)

        var radius = 0.5 * Math.sqrt(dx * dx + dy * dy + dz * dz)
        if (radius <= 0.0001 || isNaN(radius)) {
            radius = 50.0
            cx = 0
            cy = 0
            cz = 0
        }

        pivotNode.position = Qt.vector3d(cx, cy, cz)
        pivotNode.eulerRotation = Qt.vector3d(-24, 38, 0)

        var halfFovRad = (thumbCamera.fieldOfView * 0.5) * Math.PI / 180.0
        var distance = (radius / Math.sin(halfFovRad)) * 1.18

        thumbCamera.position = Qt.vector3d(0, 0, distance)
        thumbCamera.clipNear = Math.max(0.005, radius * 0.005)
        thumbCamera.clipFar = Math.max(1000.0, radius * 40.0)
    }

    View3D {
        anchors.fill: parent

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "#232834"
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        Node {
            id: pivotNode
            eulerRotation: Qt.vector3d(-24, 38, 0)

            PerspectiveCamera {
                id: thumbCamera
                position: Qt.vector3d(0, 0, 300)
                fieldOfView: 40
                clipNear: 0.1
                clipFar: 10000
            }
        }

        //三点布光，让静态缩略图更有立体感
        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: 42
            brightness: 1.35
        }

        DirectionalLight {
            eulerRotation.x: 25
            eulerRotation.y: -135
            brightness: 0.70
            color: "#d6e4ff"
        }

        DirectionalLight {
            eulerRotation.x: -80
            eulerRotation.y: 0
            brightness: 0.35
        }

        RuntimeLoader {
            id: loader
            source: offscreenRoot.modelSource

            onStatusChanged: {
                if (status === RuntimeLoader.Success) {
                    offscreenRoot.fitCameraToModel()
                    if (thumbGenerator)
                        thumbGenerator.notifyModelStatus(1, "")
                } else if (status === RuntimeLoader.Error) {
                    if (thumbGenerator)
                        thumbGenerator.notifyModelStatus(2, errorString)
                }
            }
        }
    }
}
