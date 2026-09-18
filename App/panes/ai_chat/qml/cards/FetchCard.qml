import QtQuick 2.15
import QtQuick.Controls 2.15

/**
 * FetchCard — fetch tool card.
 *
 * Layout:
 *   ┌─────────────────────────────────────────┐
 *   │ FETCH                           [Copy]  │  ← header
 *   │ fetch:https://example.com...      ✔     │  ← content + status
 *   └─────────────────────────────────────────┘
 */
Item {
    property var partData: null

    property string url: partData ? partData.content : ""
    property int    status: partData ? partData.status : 0  // 0=processing, 1=success, 2=failure

    implicitHeight: 60

    Rectangle {
        id: cardRect
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 60
        radius: 4
        color: "#1e2233"
        border.color: "#2e3548"
        border.width: 1

        // ── Header bar ──
        Rectangle {
            id: headerBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 28
            radius: 4
            color: "#262d44"
            // Square off bottom corners
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: 4
                color: parent.color
            }

            // Tool type label
            Text {
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                text: "FETCH"
                color: "#8a8a8a"
                font.pixelSize: 11
                font.family: "Consolas"
                font.bold: true
            }

            // Copy button
            ToolButton {
                id: copyBtn
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                width: 50
                height: 22

                contentItem: Text {
                    text: copyBtn._copied ? "✓" : qsTr("Copy")
                    color: copyBtn._copied ? "#4ec9b0" : "#888888"
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    radius: 3
                    color: copyBtn.hovered ? "#2e3548" : "transparent"
                }

                property bool _copied: false

                onClicked: {
                    // Copy only the URL, not the "fetch:" prefix
                    copyHelper.text = url
                    copyHelper.selectAll()
                    copyHelper.copy()
                    copyHelper.text = ""
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

        // ── Content line + status ──
        Row {
            anchors.top: headerBar.bottom
            anchors.topMargin: 4
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            height: 22
            spacing: 6

            // "fetch:" prefix
            Text {
                text: "fetch:"
                font.family: "Consolas"
                font.pixelSize: 12
                color: "#6a7a9a"
                anchors.verticalCenter: parent.verticalCenter
            }

            // URL text — single line, elided
            Text {
                anchors.left: parent.left
                anchors.leftMargin: 50
                anchors.rightMargin: 50
                anchors.right: parent.right
                text: url
                font.family: "Consolas"
                font.pixelSize: 12
                color: "#c8d6e8"
                elide: Text.ElideRight
                maximumLineCount: 1
                wrapMode: Text.NoWrap
                anchors.verticalCenter: parent.verticalCenter
            }

            // Status text (right-aligned)
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

        // Hidden TextInput for clipboard access
        TextInput {
            id: copyHelper
            visible: false
        }
    }
}
