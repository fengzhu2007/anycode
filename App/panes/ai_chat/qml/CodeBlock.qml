import QtQuick 2.15
import QtQuick.Controls 2.15

/**
 * CodeBlock — custom fenced code block with language label and copy button.
 * Used by MarkdownBody to render ``` blocks with a distinct visual style.
 */
Item {
    id: codeBlockRoot

    property string code: ""
    property string language: ""

    implicitHeight: headerBar.height + Math.min(codeFlick.contentHeight, 300) + 16

    // Background
    Rectangle {
        id: bg
        anchors.fill: parent
        radius: 6
        color: "#1e1e1e"
        border.color: "#3e3e3e"
        border.width: 1
    }

    // Header bar: language label + copy button
    Rectangle {
        id: headerBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 30
        radius: 6
        color: "#2d2d2d"
        // Square off bottom corners by overlaying a small rect
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 6
            color: parent.color
        }

        // Language label
        Text {
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            text: codeBlockRoot.language || "code"
            color: "#888888"
            font.pixelSize: 11
            font.family: "Consolas"
        }

        // Copy button
        ToolButton {
            id: copyBtn
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: 50
            height: 24

            contentItem: Text {
                text: copyBtn._copied ? "✓" : "Copy"
                color: copyBtn._copied ? "#4ec9b0" : "#888888"
                font.pixelSize: 11
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                radius: 3
                color: copyBtn.hovered ? "#3e3e3e" : "transparent"
            }

            property bool _copied: false

            onClicked: {
                copyToClipboard(codeBlockRoot.code)
                _copied = true
                copyTimer.restart()
            }
        }

        Timer {
            id: copyTimer
            interval: 1500
            onTriggered: copyBtn._copied = false
        }
    }

    // Code text (scrollable, max height 300)
    Flickable {
        id: codeFlick
        anchors.top: headerBar.bottom
        anchors.topMargin: 8
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.right: parent.right
        anchors.rightMargin: 12
        height: Math.min(contentHeight, 300)
        contentHeight: codeText.implicitHeight
        flickableDirection: Flickable.VerticalFlick
        clip: true

        TextEdit {
            id: codeText
            width: parent.width

            text: codeBlockRoot.code
            textFormat: TextEdit.PlainText
            wrapMode: TextEdit.Wrap
            readOnly: true
            selectByMouse: true
            color: "#d4d4d4"
            font.family: "Consolas"
            font.pixelSize: 12
            selectionColor: "#264f78"
            selectedTextColor: "#ffffff"
            mouseSelectionMode: TextEdit.SelectCharacters
            textMargin: 0
        }
    }

    // Hidden TextInput for clipboard access
    TextInput {
        id: clipboardHelper
        visible: false
    }

    function copyToClipboard(text) {
        clipboardHelper.text = text
        clipboardHelper.selectAll()
        clipboardHelper.copy()
        clipboardHelper.text = ""
    }
}
