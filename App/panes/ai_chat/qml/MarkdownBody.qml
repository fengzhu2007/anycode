import QtQuick 2.15
import QtQuick.Controls 2.15
import App.MD 1.0

/**
 * MarkdownBody — renders markdown text with custom code block UI.
 *
 * Parses fenced code blocks (```lang ... ```) from the markdown text and
 * renders them with CodeBlock.qml (dark background, language label, copy
 * button). Non-code segments are rendered with TextEdit + MarkdownText.
 */
Item {
    id: markdownRoot

    property var partData: null
    property string displayText: partData ? partData.content : ""
    property bool streaming: false

    // Parse displayText into segments: [{type: "text"|"code", content, language}]
    property var segments: parseSegments(displayText)

    // FIX: Use explicit height instead of mainColumn.implicitHeight
    implicitHeight: mainColumn.height + 4

    Column {
        id: mainColumn
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 6

        // Calculate height explicitly to avoid implicitHeight recalculation loop
        height: {
            var h = 0;
            for (var i = 0; i < children.length; ++i) {
                var child = children[i];
                if (child.visible) {
                    h += child.height;
                    if (h > 0) h += spacing;
                }
            }
            return h > 0 ? h - spacing : 0;
        }

        Repeater {
            model: segments

            delegate: Item {
                width: mainColumn.width
                height: modelData.type === "text" ? textSegment.implicitHeight : codeSegment.implicitHeight

                // Text segment
                TextEdit {
                    id: textSegment
                    visible: modelData.type === "text"
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8

                    text: modelData.content
                    textFormat: TextEdit.MarkdownText
                    wrapMode: TextEdit.Wrap
                    readOnly: true
                    focus: true
                    selectByMouse: true
                    selectByKeyboard: true
                    color: "#dcdcdc"
                    font.pixelSize: 12
                    selectionColor: "#0539a2"
                    selectedTextColor: "#ffffff"
                    mouseSelectionMode: TextEdit.SelectCharacters



                    onLinkActivated: function(link) {
                        if (link.indexOf("http") === 0)
                            Qt.openUrlExternally(link)
                    }

                    Component.onCompleted: {
                        if (modelData.type === "text")
                            tableStyler.attach(textDocument)
                    }
                    Component.onDestruction: {
                        if (modelData.type === "text")
                            tableStyler.detach(textDocument)
                    }
                }

                // Code block segment
                CodeBlock {
                    id: codeSegment
                    visible: modelData.type === "code"
                    anchors.left: parent.left
                    anchors.right: parent.right

                    code: modelData.content
                    language: modelData.language || ""
                }


            }
        }
    }

    MDStyler {
            id: tableStyler
            borderColor:     "#c8d1da"
            headerBg:        "#eef2f6"
            rowEvenBg:       "#ffffff"
            rowOddBg:        "#f7f9fb"
            headerTextColor: "#1f2328"
            lineHeight:120
            cellPadding: 0
            borderWidth: 0
            paused: markdownRoot.streaming
        }


    /**
     * Parse markdown text into segments of text and code blocks.
     * Fenced code blocks are detected by ``` markers.
     */
    function parseSegments(text) {
        var result = [];
        if (!text || text.length === 0) {
            result.push({type: "text", content: "", language: ""});
            return result;
        }

        var pos = 0;
        var len = text.length;

        while (pos < len) {
            // Look for next ``` at the start of a line
            var fenceStart = findFenceStart(text, pos);

            if (fenceStart < 0) {
                // No more code blocks — rest is text
                var remaining = text.substring(pos);
                if (remaining.length > 0)
                    result.push({type: "text", content: remaining, language: ""});
                break;
            }

            // Text before the fence
            if (fenceStart > pos) {
                var before = text.substring(pos, fenceStart);
                if (before.length > 0)
                    result.push({type: "text", content: before, language: ""});
            }

            // Extract language label (after ``` on the same line)
            var langEnd = text.indexOf("\n", fenceStart);
            if (langEnd < 0) langEnd = len;
            var lang = text.substring(fenceStart + 3, langEnd).trim();

            // Find closing ```
            var fenceEnd = findFenceEnd(text, langEnd);

            if (fenceEnd < 0) {
                // Unclosed fence — treat rest as code
                var code = text.substring(langEnd + 1);
                result.push({type: "code", content: code, language: lang});
                break;
            }

            // Extract code content
            var codeContent = text.substring(langEnd + 1, fenceEnd);
            result.push({type: "code", content: codeContent, language: lang});

            // Move past the closing fence line
            var afterFence = text.indexOf("\n", fenceEnd);
            pos = (afterFence >= 0) ? afterFence + 1 : len;
        }

        if (result.length === 0)
            result.push({type: "text", content: text, language: ""});

        return result;
    }

    /**
     * Find the start of a fenced code block (``` at the beginning of a line).
     * Returns the index of the first ` character, or -1 if not found.
     */
    function findFenceStart(text, fromPos) {
        var len = text.length;
        var pos = fromPos;

        // Check if text starts with ``` right at fromPos
        if (pos === 0 && text.substring(0, 3) === "```")
            return 0;

        while (pos < len) {
            var nl = text.indexOf("\n", pos);
            if (nl < 0) break;
            var lineStart = nl + 1;
            if (lineStart + 2 < len && text.substring(lineStart, lineStart + 3) === "```")
                return lineStart;
            pos = lineStart;
        }
        return -1;
    }

    /**
     * Find the closing ``` fence after the given position.
     * Returns the index of the first ` of the closing ```, or -1.
     */
    function findFenceEnd(text, fromPos) {
        var len = text.length;
        var pos = fromPos;

        while (pos < len) {
            var nl = text.indexOf("\n", pos);
            if (nl < 0) break;
            var lineStart = nl + 1;
            if (lineStart + 2 < len && text.substring(lineStart, lineStart + 3) === "```") {
                // Make sure it's just ``` (possibly with trailing whitespace)
                var afterFence = lineStart + 3;
                while (afterFence < len && (text.charAt(afterFence) === ' ' || text.charAt(afterFence) === '\t'))
                    afterFence++;
                if (afterFence >= len || text.charAt(afterFence) === '\n')
                    return lineStart;
            }
            pos = (nl >= 0) ? nl + 1 : len;
        }
        return -1;
    }
}
