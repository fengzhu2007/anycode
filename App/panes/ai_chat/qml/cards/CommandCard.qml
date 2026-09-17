import QtQuick 2.15

/**
 * CommandCard — command-line tool card (bash/cmd/powershell).
 *
 * Shows shell type badge, command content, and status indicator.
 */
Item {
    property var partData: null

    property string toolName: partData ? partData.toolName : ""
    property string shellType: partData ? partData.shellType.toUpperCase() : "SHELL"
    property string command: partData ? partData.content : ""
    property int    status: partData ? partData.status : 0  // 0=processing, 1=success, 2=failure

    implicitHeight: cardRect.height

    Rectangle {
        id: cardRect
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        anchors.top: parent.top
        height: cardCol.implicitHeight + 16
        radius: 4
        color: "#1e2a1e"
        border.color: {
            if (status === 1) return "#42b983"
            if (status === 2) return "#e74c3c"
            return "#e6a23c"
        }
        border.width: status > 0 ? 1 : 0

        // Left accent border
        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 3
            radius: 1.5
            color: {
                if (status === 1) return "#42b983"
                if (status === 2) return "#e74c3c"
                return "#e6a23c"
            }
        }

        Column {
            id: cardCol
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.leftMargin: 12
            anchors.topMargin: 8
            anchors.rightMargin: 8
            spacing: 6

            // Header: status icon + tool name + shell badge
            Row {
                width: parent.width
                spacing: 6
                height: 20

                // Status icon
                Text {
                    text: {
                        if (status === 0) return "⏳"
                        if (status === 1) return "✔"
                        if (status === 2) return "✘"
                        return "⏳"
                    }
                    font.pixelSize: 12
                    color: {
                        if (status === 0) return "#e6a23c"
                        if (status === 1) return "#42b983"
                        if (status === 2) return "#e74c3c"
                        return "#e6a23c"
                    }
                    anchors.verticalCenter: parent.verticalCenter
                }

                // Tool name
                Text {
                    text: toolName
                    font.pixelSize: 12
                    font.bold: true
                    color: "#c8d6c8"
                    anchors.verticalCenter: parent.verticalCenter
                }

                // Shell type badge
                Rectangle {
                    width: shellText.implicitWidth + 8
                    height: 18
                    radius: 3
                    color: "#333333"
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        id: shellText
                        anchors.centerIn: parent
                        text: shellType
                        font.pixelSize: 10
                        font.bold: true
                        color: "#e6a23c"
                    }
                }

                // Status text (when completed)
                Item { width: 1; height: 1 }

                Text {
                    visible: status > 0
                    text: status === 1 ? "Success" : "Failure"
                    font.pixelSize: 11
                    color: status === 1 ? "#42b983" : "#e74c3c"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // Command content (shown while processing)
            TextEdit {
                visible: status === 0
                width: parent.width
                text: command
                wrapMode: TextEdit.Wrap
                textFormat: TextEdit.PlainText
                font.family: "Consolas"
                font.pixelSize: 12
                color: "#c8d6c8"
                readOnly: true
                selectByMouse: true
                selectionColor: "#3399ff"
                selectedTextColor: "#ffffff"
                mouseSelectionMode: TextEdit.SelectCharacters
            }
        }
    }
}
