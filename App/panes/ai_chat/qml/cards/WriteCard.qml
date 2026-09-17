import QtQuick 2.15

/**
 * WriteCard — file write tool card.
 *
 * Shows target file path and write status.
 */
Item {
    property var partData: null

    property string toolName: partData ? partData.toolName : "Write"
    property string content: partData ? partData.content : ""
    property int    status: partData ? partData.status : 0

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
        border.color: status === 1 ? "#42b983" : (status === 2 ? "#e74c3c" : "#e6a23c")
        border.width: status > 0 ? 1 : 0

        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 3
            radius: 1.5
            color: status === 1 ? "#42b983" : (status === 2 ? "#e74c3c" : "#e6a23c")
        }

        Column {
            id: cardCol
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.leftMargin: 12
            anchors.topMargin: 8
            anchors.rightMargin: 8
            spacing: 4

            Row {
                width: parent.width
                spacing: 6
                height: 20

                Text {
                    text: status === 0 ? "⏳" : (status === 1 ? "✔" : "✘")
                    font.pixelSize: 12
                    color: status === 0 ? "#e6a23c" : (status === 1 ? "#42b983" : "#e74c3c")
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: "✏️ " + toolName
                    font.pixelSize: 12
                    font.bold: true
                    color: "#c8d6c8"
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    visible: status > 0
                    text: status === 1 ? "Success" : "Failure"
                    font.pixelSize: 11
                    color: status === 1 ? "#42b983" : "#e74c3c"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // Content preview (truncated)
            TextEdit {
                visible: status === 0
                width: parent.width
                text: {
                    var c = content
                    return c.length > 200 ? c.substring(0, 200) + "..." : c
                }
                wrapMode: TextEdit.Wrap
                textFormat: TextEdit.PlainText
                font.family: "Consolas"
                font.pixelSize: 12
                color: "#8899aa"
                readOnly: true
                selectByMouse: true
                selectionColor: "#3399ff"
                selectedTextColor: "#ffffff"
                mouseSelectionMode: TextEdit.SelectCharacters
            }
        }
    }
}
