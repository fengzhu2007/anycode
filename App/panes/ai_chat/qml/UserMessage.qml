import QtQuick 2.15


/**
 * UserMessage — right-aligned user bubble with solid background.
 */
Item {
    property string messageType: "user"
    property string content: ""
    property string thinking: ""
    property bool   streaming: false
    property var    parts: []

    width: parent.width
    implicitHeight: bubble.implicitHeight + 8

    Rectangle {
        id: bubble
        anchors.right: parent.right
        // Fit the text's natural (unwrapped) width, capped to the item
        // width so long messages wrap instead of overflowing.
        width: Math.min(textItem.implicitWidth + 24, parent.width)
        implicitHeight: textItem.implicitHeight + 16
        radius: 8
        color: "#2f362f"

        TextEdit {
            id: textItem
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.right: parent.right
            anchors.rightMargin: 12
            anchors.top: parent.top
            anchors.topMargin: 8
            text: content
            wrapMode: TextEdit.Wrap
            textFormat: TextEdit.MarkdownText
            color: "#eeeeee"
            font.pixelSize: 12
            readOnly: true
            selectByKeyboard: true
            selectByMouse: true
            selectionColor: "#0539a2"
            selectedTextColor: "#ffffff"
            mouseSelectionMode: TextEdit.SelectCharacters
            renderType: TextEdit.NativeRendering

            // Suppress cursor change on hover
            cursorDelegate: Item {}
        }
    }
}
