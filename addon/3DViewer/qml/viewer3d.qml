import QtQuick
import QtQuick3D
import QtQuick3D.AssetUtils
import QtQuick3D.Helpers

Item {
    id: root

    readonly property bool sceneReady: viewer ? viewer.active : false

    /**
     * Fit-to-View：根据 RuntimeLoader.bounds 计算包围球半径与视口纵横比，
     * 得到既不水平裁切也不垂直裁切的相机距离，并把旋转中心对齐到包围盒中心。
     */
    function fitCameraToModel() {
        if (!viewer || loader.status !== RuntimeLoader.Success)
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
            dx = 100
            dy = 100
            dz = 100
        }

        originNode.position = Qt.vector3d(cx, cy, cz)
        originNode.eulerRotation = Qt.vector3d(-22, 35, 0)

        var vFovRad = cameraNode.fieldOfView * Math.PI / 180.0
        var aspect = (view3d.width > 0 && view3d.height > 0) ? (view3d.width / view3d.height) : 1.0
        var hFovRad = 2.0 * Math.atan(Math.tan(vFovRad * 0.5) * aspect)
        var effectiveHalfFov = 0.5 * Math.min(vFovRad, hFovRad)

        var distance = (radius / Math.sin(Math.max(0.15, effectiveHalfFov))) * 1.15
        cameraNode.position = Qt.vector3d(0, 0, distance)
        cameraNode.clipNear = Math.max(0.001, radius * 0.004)
        cameraNode.clipFar = Math.max(2000.0, radius * 50.0)

        viewer.reportModelBounds(dx, dy, dz, radius, distance)
    }

    View3D {
        id: view3d
        anchors.fill: parent
        visible: root.sceneReady

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "#1e222a"
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High

            debugSettings: DebugSettings {
                wireframeEnabled: viewer ? viewer.wireframeMode : false
            }
        }

        Node {
            id: originNode
            eulerRotation: Qt.vector3d(-22, 35, 0)

            PerspectiveCamera {
                id: cameraNode
                position: Qt.vector3d(0, 0, 300)
                fieldOfView: 45
                clipNear: 0.1
                clipFar: 10000
            }
        }

        //三点布光，保证任意角度都有足够明暗层次
        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: 40
            brightness: 1.25
        }

        DirectionalLight {
            eulerRotation.x: 35
            eulerRotation.y: -140
            brightness: 0.65
            color: "#dce8ff"
        }

        DirectionalLight {
            eulerRotation.x: -80
            eulerRotation.y: 0
            brightness: 0.35
        }

        AxisHelper {
            visible: viewer ? viewer.showGrid : true
            enableXYGrid: false
            enableYZGrid: false
            enableXZGrid: true
            gridColor: "#384152"
            gridOpacity: 0.45
        }

        RuntimeLoader {
            id: loader
            source: viewer ? viewer.modelSource : ""

            onStatusChanged: {
                if (!viewer)
                    return
                if (status === RuntimeLoader.Success) {
                    root.fitCameraToModel()
                    viewer.notifyModelStatus(1, "")
                } else if (status === RuntimeLoader.Error) {
                    viewer.notifyModelStatus(2, errorString)
                }
            }

            onBoundsChanged: {
                if (viewer && status === RuntimeLoader.Success)
                    root.fitCameraToModel()
            }
        }

        OrbitCameraController {
            anchors.fill: parent
            origin: originNode
            camera: cameraNode
            panEnabled: true
            xSpeed: 0.35
            ySpeed: 0.35
        }
    }

    //操作提示
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 10
        width: hintText.implicitWidth + 24
        height: 26
        radius: 6
        color: "#aa181c24"
        visible: root.sceneReady && loader.status === RuntimeLoader.Success

        Text {
            id: hintText
            anchors.centerIn: parent
            text: qsTr("Left drag: orbit   |   Right drag: pan   |   Wheel: zoom")
            color: "#9da5b4"
            font.pixelSize: 12
        }
    }

    //占位 / 错误提示
    Rectangle {
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.8, 420)
        height: Math.max(96, messageText.implicitHeight + 40)
        radius: 8
        color: "#d921252b"
        border.color: "#3e4451"
        border.width: 1
        visible: messageText.text.length > 0

        Text {
            id: messageText
            anchors.fill: parent
            anchors.margins: 16
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
            text: viewer ? viewer.overlayMessage : ""
            color: "#dcdfe4"
            font.pixelSize: 13
        }
    }
}
