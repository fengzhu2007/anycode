import QtQuick 2.15

/**
 * ThinkingBlock — collapsible thinking content display.
 *
 * Uses only Text components (reliable implicitHeight in ListView delegate).
 * Collapsed: single-line preview with elide + "thinking..."
 * Expanded: full wrapped text
 * Click to toggle. Right-click to copy.
 */
Item {
    id: thinkingRoot

    property var partData: null
    property bool expanded: false
    property string thinkText: partData ? partData.content : ""

    implicitHeight: expanded ? Math.max(icon.height, fullTextLabel.implicitHeight) + 8
                             : Math.max(icon.height, previewLabel.implicitHeight) + 8
    clip: true

    Image {
        id: icon
        anchors.left: parent.left
        anchors.top: parent.top
        source: "qrc:/Resource/icons/dark/IntellisenseLightBulb_16x.svg"
        width: 12
        height: 12
    }

    // Collapsed: single-line preview with elide
    Text {
        id: previewLabel
        visible: !expanded
        anchors.left: parent.left
        anchors.leftMargin: 18
        anchors.rightMargin: 30
        anchors.right: parent.right
        anchors.top: parent.top
        text: thinkText
        elide: Text.ElideRight
        maximumLineCount: 1
        wrapMode: Text.NoWrap
        color: "#888888"
        font.pixelSize: 12

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: expanded = true


        }
    }

    // Expanded: full wrapped text (always laid out, opacity controls visibility)
    TextEdit {
        id: fullTextLabel
        visible: expanded
        anchors.left: parent.left
        anchors.leftMargin: 18
        anchors.rightMargin: 30
        anchors.right: parent.right
        anchors.top: parent.top
        text: thinkText
        wrapMode: Text.Wrap
        textFormat: Text.PlainText
        color: "#888888"
        font.pixelSize: 12
        opacity: expanded ? 1 : 0
        readOnly: true
        selectByMouse: true
        selectionColor: "#0539a2"
        selectedTextColor: "#ffffff"
        mouseSelectionMode: TextEdit.SelectCharacters
        renderType: TextEdit.NativeRendering

        // Suppress cursor change on hover
        cursorDelegate: Item {}
    }

    // Toggle arrow at top-right
    MouseArea {
        anchors.right: parent.right
        anchors.top: parent.top
        width: 20
        height: 14
        cursorShape: Qt.PointingHandCursor
        onClicked: expanded = !expanded

        Text {
            text: expanded ? "▲" : "▼"
            font.pixelSize: 10
            color: "#888888"
            anchors.centerIn: parent
        }
    }
}
