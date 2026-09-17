import QtQuick 2.15

/**
 * PermissionCard — permission request card with action buttons.
 *
 * Shows tool name, detail, and Allow Once / Allow Always / Reject buttons.
 * After reply, shows the chosen action.
 */
Item {
    property var partData: null
    property string messageType: "permission"
    property string content: ""

    // Parse permission data — from partData (embedded) or content (standalone)
    property string requestId: partData ? partData.permissionRequestId : ""
    property string reply: partData ? partData.permissionReply : ""
    property var permData: {
        try {
            var text = (partData ? partData.content : "") || content
            if (!text) return ({})
            return JSON.parse(text)
        } catch(e) {
            return ({})
        }
    }
    property string toolName: permData.toolName || ""
    property string detail: permData.detail || ""

    implicitHeight: mainRect.height

    Rectangle {
        id: mainRect
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        anchors.top: parent.top
        height: mainCol.implicitHeight + 16
        radius: 6
        color: "#2a2a2a"
        border.color: "#3a3a3a"
        border.width: 1

        Column {
            id: mainCol
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            anchors.topMargin: 8
            spacing: 8

            // Permission description
            Column {
                spacing: 4
                width: parent.width

                Text {
                    text: "⚠ Permission Request"
                    font.pixelSize: 12
                    font.bold: true
                    color: "#e6a23c"
                }
                Text {
                    text: "Tool [" + toolName + "] is requesting permission"
                    font.pixelSize: 12
                    color: "#aaaaaa"
                }
                Text {
                    text: detail
                    font.pixelSize: 12
                    color: "#888888"
                    width: parent.width
                    wrapMode: Text.Wrap
                }
            }

            // Buttons (visible only if no reply yet)
            Row {
                visible: reply.length === 0 && requestId.length > 0
                width: parent.width
                spacing: 8
                layoutDirection: Qt.RightToLeft

                PermButton {
                    text: "Reject"
                    bgColor: "#6c757d"
                    onClicked: partData.replyPermission("reject")
                }
                PermButton {
                    text: "Allow Always"
                    bgColor: "#1a759f"
                    onClicked: partData.replyPermission("always")
                }
                PermButton {
                    text: "Allow Once"
                    bgColor: "#2d6a4f"
                    onClicked: partData.replyPermission("once")
                }
            }

            // Reply status (shown after reply)
            Text {
                visible: reply.length > 0
                font.pixelSize: 11
                font.bold: true
                color: {
                    if (reply === "once" || reply === "always") return "#2d6a4f"
                    if (reply === "reject") return "#e74c3c"
                    return "#888888"
                }
                text: {
                    if (reply === "once")    return "✔ Allowed (once)"
                    if (reply === "always")  return "✔ Allowed (always)"
                    if (reply === "reject")  return "✘ Rejected"
                    return ""
                }
            }
        }
    }

    // Reusable button component
    component PermButton: Rectangle {
        property string text: ""
        property string bgColor: "#333333"
        signal clicked()

        width: btnText.implicitWidth + 20
        height: 28
        radius: 4
        color: bgColor

        Text {
            id: btnText
            anchors.centerIn: parent
            text: parent.text
            font.pixelSize: 11
            color: "#ffffff"
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: parent.clicked()
        }
    }
}
