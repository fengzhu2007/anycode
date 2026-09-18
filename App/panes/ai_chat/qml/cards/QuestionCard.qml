import QtQuick 2.15

/**
 * QuestionCard — question/ask-user tool card.
 *
 * Shows the question being asked to the user.
 */
Item {
    property var partData: null

    property string toolName: partData ? partData.toolName : "Question"
    property string question: partData ? partData.content : ""
    property int    status: partData ? partData.status : 0

    implicitHeight: cardRect.height

    Rectangle {
        id: cardRect
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        anchors.top: parent.top
        height: {
            var h = 0;
            for (var i = 0; i < cardCol.children.length; ++i) {
                var c = cardCol.children[i];
                if (c.visible) { h += c.height; if (h > 0) h += cardCol.spacing; }
            }
            return (h > 0 ? h - cardCol.spacing : 0) + 16;
        }
        radius: 4
        color: "#2a1e2a"
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
                    text: status === 0 ? "❓" : (status === 1 ? "✔" : "✘")
                    font.pixelSize: 12
                    color: status === 0 ? "#e6a23c" : (status === 1 ? "#42b983" : "#e74c3c")
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: toolName
                    font.pixelSize: 12
                    font.bold: true
                    color: "#d6a0d6"
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    visible: status > 0
                    text: status === 1 ? "Answered" : "Rejected"
                    font.pixelSize: 11
                    color: status === 1 ? "#42b983" : "#e74c3c"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // Question text
            TextEdit {
                visible: status === 0
                width: parent.width
                text: question
                wrapMode: TextEdit.Wrap
                textFormat: TextEdit.PlainText
                font.pixelSize: 12
                color: "#aa88aa"
                readOnly: true
                selectByMouse: true
                selectionColor: "#3399ff"
                selectedTextColor: "#ffffff"
                mouseSelectionMode: TextEdit.SelectCharacters
            }

            // Answer (when completed)
            TextEdit {
                visible: status === 1
                width: parent.width
                text: question
                wrapMode: TextEdit.Wrap
                textFormat: TextEdit.PlainText
                font.pixelSize: 12
                color: "#88aa88"
                readOnly: true
                selectByMouse: true
                selectionColor: "#3399ff"
                selectedTextColor: "#ffffff"
                mouseSelectionMode: TextEdit.SelectCharacters
            }
        }
    }
}
