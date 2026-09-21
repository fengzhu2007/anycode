import QtQuick 2.15

/**
 * TodoCard — todo_write plan card.
 *
 * Renders the task plan published via todo.created / todo.updated SSE events.
 * partData carries: todoTitle, todoTasks [{id, content, status, output}].
 * Task status is updated live by PartObject::setTodoTasks (todoTasksChanged).
 *   pending   → hollow gray circle
 *   running   → pulsing blue circle
 *   completed → green check, strikethrough text
 *   failed    → red cross
 */
Item {
    property var partData: null

    property string planTitle: partData ? partData.todoTitle : ""
    property var tasks: partData ? partData.todoTasks : []

    function statusIcon(s) {
        if (s === "completed") return "\u2714"   // ✔
        if (s === "failed")    return "\u2718"   // ✘
        if (s === "running")   return "\u25D0"   // ◐
        return "\u25CB"                          // ○
    }
    function statusColor(s) {
        if (s === "completed") return "#42b983"
        if (s === "failed")    return "#e74c3c"
        if (s === "running")   return "#4a9eff"
        return "#8a8f94"
    }
    function doneCount() {
        var n = 0
        for (var i = 0; i < tasks.length; ++i)
            if (tasks[i].status === "completed") ++n
        return n
    }

    property bool allDone: tasks.length > 0 && doneCount() === tasks.length

    implicitHeight: cardRect.height

    Rectangle {
        id: cardRect
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        anchors.top: parent.top
        // Explicit height calculation — Column.implicitHeight in a ListView
        // delegate causes layout recalculation loops (see other cards).
        // Zero-height children (the Repeater itself) are skipped.
        height: {
            var h = 0;
            for (var i = 0; i < cardCol.children.length; ++i) {
                var c = cardCol.children[i];
                if (c.visible && c.height > 0) {
                    h += c.height;
                    if (h > 0) h += cardCol.spacing;
                }
            }
            return (h > 0 ? h - cardCol.spacing : 0) + 16;
        }
        radius: 4
        color: "#1e2233"
        border.color: "#2e3548"
        border.width: 1

        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 3
            radius: 1.5
            color: allDone ? "#42b983" : "#4a9eff"
        }

        Column {
            id: cardCol
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.leftMargin: 12
            anchors.topMargin: 8
            anchors.rightMargin: 8
            spacing: 6

            // ---- header: icon + title + progress ----
            Row {
                width: parent.width
                spacing: 6
                height: 20

                Text {
                    text: allDone ? "\u2705" : "\uD83D\uDCCB"   // ✅ / 📋
                    font.pixelSize: 12
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: planTitle.length > 0 ? planTitle : "To-dos"
                    font.pixelSize: 12
                    font.bold: true
                    color: "#d0d4d9"
                    elide: Text.ElideRight
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: doneCount() + "/" + tasks.length
                    font.pixelSize: 11
                    color: allDone ? "#42b983" : "#8a8f94"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // ---- task rows ----
            Repeater {
                model: tasks

                delegate: Item {
                    width: parent ? parent.width : 300
                    height: Math.max(iconText.height, taskText.height)

                    Text {
                        id: iconText
                        anchors.left: parent.left
                        anchors.top: parent.top
                        width: 16
                        text: statusIcon(modelData.status)
                        color: statusColor(modelData.status)
                        font.pixelSize: 12

                        SequentialAnimation on opacity {
                            running: modelData.status === "running"
                            loops: Animation.Infinite
                            NumberAnimation { to: 0.25; duration: 500 }
                            NumberAnimation { to: 1.0; duration: 500 }
                        }
                    }

                    Text {
                        id: taskText
                        anchors.left: iconText.right
                        anchors.leftMargin: 6
                        anchors.right: parent.right
                        anchors.top: parent.top
                        text: modelData.content
                        wrapMode: Text.Wrap
                        font.pixelSize: 12
                        font.strikeout: modelData.status === "completed"
                        color: modelData.status === "completed" ? "#6b7680"
                             : (modelData.status === "pending" ? "#9aa3ab" : "#d0d4d9")
                    }
                }
            }
        }
    }
}
