import QtQuick 2.15

/**
 * ThinkingBlock — collapsible thinking content display.
 *
 * Collapsed: lightweight Text preview (single line, elide).
 * Expanded: TextEdit loaded on-demand via Loader (selectByMouse support).
 *
 * The TextEdit is NOT created until the user expands the block.
 * This eliminates TextEdit overhead during streaming — no implicitHeight
 * recalculation, no QTextDocument allocation, no layout propagation.
 */
Item {
    id: thinkingRoot

    property var partData: null
    property bool expanded: false
    property string thinkText: partData ? partData.content : ""

    implicitHeight: expanded && expandedLoader.item
                        ? expandedLoader.item.implicitHeight
                        : previewLabel.implicitHeight
    clip: true

    Image {
        id: icon
        source: "qrc:/Resource/icons/dark/LightBulb.png"
        anchors.top: parent.top
        width:12
        height:12
    }

    // Collapsed: single-line preview with elide
    Text {
        id: previewLabel
        visible: !expanded
        anchors.left: parent.left
        anchors.leftMargin: 18
        anchors.rightMargin: 30
        anchors.bottomMargin: 12
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

    // Expanded: TextEdit loaded on demand.
    // No TextEdit exists during streaming — zero overhead.
    Loader {
        id: expandedLoader
        visible: expanded
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        active: expanded
        anchors.leftMargin: 18
        anchors.rightMargin: 30

        sourceComponent: Component {
            TextEdit {
                text: thinkText
                wrapMode: TextEdit.Wrap
                textFormat: TextEdit.PlainText
                color: "#888888"
                font.pixelSize: 12
                readOnly: true
                selectByMouse: true
                selectionColor: "#0539a2"
                selectedTextColor: "#ffffff"
                mouseSelectionMode: TextEdit.SelectCharacters
                renderType: TextEdit.NativeRendering
                cursorDelegate: Item {}
            }
        }
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
