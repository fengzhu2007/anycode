import QtQuick 2.15

/**
 * ReadCard — read tool as simple inline text.
 *
 * Format: "read:filename.ext offset limit"
 */
Item {
    property var partData: null
    property string content: partData ? partData.content : ""

    implicitHeight: contentText.implicitHeight

    Text {
        id: contentText
        anchors.left: parent.left
        anchors.right: parent.right
        text: content
        font.family: "Consolas"
        font.pixelSize: 12
        color: "#9a9a9a"
        wrapMode: Text.NoWrap
        elide: Text.ElideRight
        maximumLineCount: 1
    }
}
