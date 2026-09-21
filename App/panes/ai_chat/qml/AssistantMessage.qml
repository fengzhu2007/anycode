import QtQuick 2.15

/**
 * AssistantMessage — renders all assistant/system/event/error content.
 *
 * Iterates over the parts array and uses Loader to dynamically load
 * the appropriate component for each part type:
 *   "thinking"   → ThinkingBlock.qml
 *   "text"       → MarkdownBody.qml
 *   "tool"       → cards/<ToolType>Card.qml (via Loader)
 *   "permission" → PermissionCard.qml
 */
Item {
    property string messageType: "assistant"
    property string content: ""
    property string thinking: ""
    property bool   streaming: false
    property var    parts: []

    // FIX: Use Column's explicit height instead of implicitHeight to prevent
    // layout recalculation loops in ListView delegate
    implicitHeight: mainColumn.height + 8

    Column {
        id: mainColumn
        width: parent.width
        spacing: 8
        // Calculate height explicitly to avoid implicitHeight recalculation
        height: {
            var h = 0;
            for (var i = 0; i < children.length; ++i) {
                var child = children[i];
                if (child.visible) {
                    h += child.height;
                    if (h > 0) h += spacing;  // Add spacing before this child (except first)
                }
            }
            return h > 0 ? h - spacing : 0;  // Remove last spacing
        }

        Repeater {
            model: parts
            delegate: Loader {
                id: partLoader
                width: mainColumn.width
                height: item ? item.implicitHeight : 0
                // modelData is a PartObject (QObject with Q_PROPERTY)
                property var part: modelData

                source: {
                    if (!part) return ""
                    var pt = part.partType
                    if (pt === "thinking") return "ThinkingBlock.qml"
                    if (pt === "text")     return "MarkdownBody.qml"
                    if (pt === "permission") return "PermissionCard.qml"
                    if (pt === "tool") {
                        // Map tool type to card component
                        var tt = part.toolType || ""
                        switch (tt) {
                            case "bash":
                            case "cmd":
                            case "powershell":
                            case "shell":
                                return "cards/CommandCard.qml"
                            case "read":
                                return "cards/ReadCard.qml"
                            case "write":
                                return "cards/WriteCard.qml"
                            case "edit":
                                return "cards/EditCard.qml"
                            case "grep":
                                return "cards/GrepCard.qml"
                            case "glob":
                                return "cards/GlobCard.qml"
                            case "fetch":
                                return "cards/FetchCard.qml"
                            case "question":
                                return "cards/QuestionCard.qml"
                            case "todo_write":
                                return "cards/TodoCard.qml"
                            default:
                                // Command-line fallback for isCommand
                                if (part.isCommand) return "cards/CommandCard.qml"
                                return "cards/ReadCard.qml"
                        }
                    }
                    return ""
                }

                // Pass part data to the loaded component
                onLoaded: {
                    if (item && part) {
                        item.partData = part
                    }
                    // Only pass streaming to the last part (the one actively streaming)
                    if (item && "streaming" in item && index === parts.length - 1) {
                        item.streaming = streaming
                    }
                }

                // Keep partData in sync when part properties change
                Binding {
                    target: partLoader.item
                    property: "partData"
                    value: part
                    when: partLoader.item !== null
                }

                // Only the last part receives streaming state.
                // When parts array grows, previous last part automatically gets false.
                Binding {
                    target: partLoader.item
                    property: "streaming"
                    value: (index === parts.length - 1) ? streaming : false
                    when: partLoader.item !== null && ("streaming" in partLoader.item)
                }
            }
        }

        // Streaming indicator at the bottom
        Row {
            visible: streaming
            leftPadding: 0
            spacing: 4
            height: 20

            Text {
                text: "▍"
                font.pixelSize: 14
                color: "#9a9a9a"

                SequentialAnimation on opacity {
                    running: streaming
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.0; duration: 500 }
                    NumberAnimation { to: 1.0; duration: 500 }
                }
            }
        }
    }
}
