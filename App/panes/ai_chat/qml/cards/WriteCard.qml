import QtQuick 2.15

/**
 * WriteCard — file write tool card.
 *
 * Layout:
 *   ┌─────────────────────────────────────────┐
 *   │ filename.cpp              +10       ✔   │
 *   └─────────────────────────────────────────┘
 */
Item {
    property var partData: null

    property string filePath: partData ? partData.content : ""
    property string fileName: {
        var parts = filePath.split(/[\/\\]/);
        return parts[parts.length - 1] || filePath;
    }
    property int linesAdded: partData ? partData.linesAdded : 0
    property int linesRemoved: partData ? partData.linesRemoved : 0
    property int status: partData ? partData.status : 0

    implicitHeight: 32

    Rectangle {
        id: cardRect
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 32
        radius: 4
        color: "#1e2233"
        border.color: "#2e3548"
        border.width: 1

        Row {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            height: 20
            spacing: 8

            // Filename (clickable)
            Text {
                id: fileNameText
                text: fileName || "write"
                anchors.left: parent.left
                anchors.right:parent.right
                anchors.rightMargin: 80
                font.pixelSize: 12
                color: "#c8d6c8"
                anchors.verticalCenter: parent.verticalCenter
                elide: Text.ElideMiddle
                maximumLineCount: 1

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (partData) partData.openFile();
                    }
                }
            }

            // Spacer

            // Lines added
            Text {
                id: addedText
                anchors.right: removedText.left
                text: "+" + linesAdded
                font.pixelSize: 11
                color: "#42b983"
                anchors.verticalCenter: parent.verticalCenter
                visible: linesAdded > 0
            }

            // Lines removed
            Text {
                id: removedText
                anchors.right: statusText.left
                text: "-" + linesRemoved
                font.pixelSize: 11
                color: "#e74c3c"
                anchors.verticalCenter: parent.verticalCenter
                visible: linesRemoved > 0
            }

            // Status
            Text {
                id: statusText
                anchors.right: parent.right
                text: {
                    if (status === 0) return qsTr("Executing")
                    if (status === 1) return qsTr("Success")
                    if (status === 2) return qsTr("Failed")
                    return qsTr("Executing")
                }
                font.pixelSize: 11
                color: {
                    if (status === 0) return "#e6a23c"
                    if (status === 1) return "#42b983"
                    if (status === 2) return "#e74c3c"
                    return "#e6a23c"
                }
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
}
