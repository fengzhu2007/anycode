import QtQuick 2.15
import QtQuick.Controls 2.15

/**
 * MainChatView — QML root for the AI chat message list.
 *
 * Expected context properties (set from C++ before setSource):
 *   messageModel : QmlMessageModel
 *
 * Scroll behavior:
 *  - History load-more fires only when the list rests exactly at the top
 *    (atYBeginning), rate-limited by loadMoreCooldown. This breaks the
 *    previous "prepend → reset → contentY<50 → load again" storm.
 *  - When history is prepended (messageModel.aboutToPrependMessages), the
 *    previously topmost message is re-anchored instead of jumping to the
 *    bottom, and auto-scroll is suppressed for that count change.
 *  - Dragging away from the bottom pauses auto-follow; reaching the bottom
 *    (or an explicit scrollToBottom()) re-enables it.
 */
Item {
    id: root

    property alias model: listView.model
    property bool autoScroll: true
    // >0 while a history prepend is being applied — suppresses auto-scroll
    property int prependPending: 0

    Rectangle {
        anchors.fill: parent
        color: themeBgColor || "#2b2b2b"
        z: -1
    }

    function scrollToBottom() {
        root.autoScroll = true
        listView.positionViewAtEnd()
    }

    // Streaming follow-up: only follow when the user hasn't scrolled away
    function autoFollow() {
        if (!root.autoScroll) return
        // Follow only when the viewport is already close to the bottom —
        // a user who scrolled up to read (drag, wheel or scrollbar) is
        // not yanked back mid-stream.
        if (listView.contentHeight - listView.contentY - listView.height > 400)
            return
        listView.positionViewAtEnd()
    }

    ListView {
        id: listView
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        anchors.topMargin: 8
        anchors.bottomMargin: 8
        model: messageModel
        spacing: 4
        clip: true
        cacheBuffer: 400
        pixelAligned: false

        // Smooth scroll
        boundsBehavior: Flickable.StopAtBounds
        flickDeceleration: 2000
        maximumFlickVelocity: 4000

        // Auto-scroll only for bottom appends (new messages); history
        // prepends are re-anchored via prependPending below.
        onCountChanged: {
            if (root.prependPending > 0) return
            if (root.autoScroll && listView.count > 0) {
                // Defer to next frame so the new delegate has its height
                Qt.callLater(function() {
                    listView.positionViewAtEnd()
                })
            }
        }

        delegate: MessageDelegate {
            width: listView.width
        }

        onContentYChanged: {
            // Re-enable auto-follow once the user returns to the bottom
            if (!root.autoScroll && listView.atYEnd)
                root.autoScroll = true

            // Load older history only when resting exactly at the top and
            // not within the cooldown window. atYBeginning (instead of a
            // raw contentY threshold) avoids firing during programmatic
            // layout transients and model resets.
            if (listView.atYBeginning && listView.count > 0
                    && !loadMoreCooldown.running) {
                loadMoreCooldown.restart()
                if (typeof messageModel !== 'undefined' && messageModel.requestLoadMore)
                    messageModel.requestLoadMore()
            }
        }

        // User dragging away from the bottom pauses auto-follow
        onDragStarted: {
            if (!listView.atYEnd)
                root.autoScroll = false
        }

        // Rate limiter for load-more requests
        Timer {
            id: loadMoreCooldown
            interval: 800
        }

        // Attached scrollbar. Re-parenting it to root (instead of the list)
        // disables the Flickable's automatic geometry management, so its
        // placement comes from the anchors below — while position, size and
        // drag interaction stay in sync with the list.
        ScrollBar.vertical: ScrollBar {
            id: vBar
            parent: root
            anchors.top: listView.top
            anchors.bottom: listView.bottom
            anchors.right: parent.right
            policy: ScrollBar.AsNeeded

            // Dragging the handle away from the bottom pauses auto-follow,
            // same as dragging the list itself.
            onPositionChanged: {
                if (vBar.pressed && !listView.atYEnd)
                    root.autoScroll = false
            }
        }
    }

    // Re-anchor the previously topmost message after history prepends so
    // the viewport neither jumps to the new rows nor to the bottom.
    Connections {
        target: messageModel
        function onAboutToPrependMessages(count) {
            root.prependPending = count
            // Wait until the new rows exist and the layout settles, then
            // place old row 0 (now at index `count`) back at the top.
            Qt.callLater(function() {
                if (root.prependPending > 0 && root.prependPending < listView.count)
                    listView.positionViewAtIndex(root.prependPending, ListView.Beginning)
                root.prependPending = 0
            })
        }
    }

}
