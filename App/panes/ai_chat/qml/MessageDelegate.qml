import QtQuick 2.15

/**
 * MessageDelegate — routes each message to the appropriate component
 * based on messageType role, using a single Loader with direct bindings.
 *
 * IMPORTANT: parts are NOT bound via `model.parts` (that would re-create
 * the array on every scroll, causing Repeater to rebuild all tool cards).
 * Instead, parts are set once in onLoaded and refreshed only when the
 * composition actually changes (see sameParts below). PartObject instances
 * are reused in place by the model, so their own NOTIFY signals keep the
 * cards updated — no array reassignment, no Repeater reset per delta.
 *
 * TEST MODE — activated by the `messageTestMode` context property (set in
 * SessionPageWidget::setupQmlView). Every row renders a fixed-height (80px)
 * empty Rectangle: no Text, no parts, no Loaders, no model content access.
 * Purpose: isolate the scroll-freeze root cause —
 *   freeze gone  → problem lives in the rendering pipeline
 *                  (MarkdownBody / TextEdit markdown layout / tool cards)
 *   freeze stays → problem lives in the view/model layer
 *                  (ListView itself, load-more loop, QQuickWidget, or the
 *                   variable-height delegate layout this stub exercises)
 * Set messageTestMode to false to restore normal rendering.
 */
Item {
    id: delegateRoot
    width: ListView.view ? ListView.view.width : 400

    property bool testMode: (typeof messageTestMode !== 'undefined') && messageTestMode === true

    // Scalar bindings — cheap, no object creation. Guarded by testMode so
    // only the stub (which reads model.content directly) touches model
    // data; the whole Loader path stays inactive.
    property string bindMessageType: testMode ? "" : model.messageType
    property string bindContent: testMode ? "" : model.content
    property string bindThinking: testMode ? "" : (model.thinking || "")
    property bool   bindStreaming: testMode ? false : (model.streaming || false)
    property int    bindPartsVersion: testMode ? 0 : (model.partsVersion || 0)

    implicitHeight: testMode ? stubRect.height : loader.implicitHeight

    // ---- TEST MODE stub: content + thinking (both NoWrap) with adaptive
    // height. AVOID Column.implicitHeight — use explicit layout to prevent
    // QML layout recalculation loops. ----
    Rectangle {
        id: stubRect
        visible: delegateRoot.testMode
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 20 + (stubThinking.visible ? stubThinking.implicitHeight + 6 : 0) + stubText.implicitHeight
        color: (index % 2 === 0) ? "#323639" : "#3a3f44"
        border.color: "#555b61"
        border.width: 1

        Text {
            id: stubThinking
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 10
            text: model.thinking || ""
            visible: text.length > 0
            elide: Text.ElideRight
            color: "#8fa3b0"
            font.pixelSize: 12
            font.italic: true
            wrapMode: Text.NoWrap
        }

        Text {
            id: stubText
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: stubThinking.visible ? stubThinking.bottom : parent.top
            anchors.topMargin: stubThinking.visible ? 16 : 10  // 10 base + 6 spacing
            text: model.content || ""
            elide: Text.ElideRight
            color: "#d0d0d0"
            font.pixelSize: 13
            wrapMode: Text.NoWrap
        }
    }

    // ---- Normal rendering path (everything below is inactive in test mode) ----

    // Last array handed to the loaded item (kept by reference so an
    // unchanged composition reassigns the SAME array value and the
    // Repeater doesn't reset)
    property var cachedParts: []

    function sameParts(a, b) {
        if (a === b) return true
        if (!a || !b || a.length !== b.length) return false
        for (var i = 0; i < a.length; ++i)
            if (a[i] !== b[i]) return false
        return true
    }

    Loader {
        id: loader
        width: parent.width
        active: !delegateRoot.testMode
        visible: !delegateRoot.testMode

        source: {
            if (delegateRoot.testMode) return ""
            switch (delegateRoot.bindMessageType) {
                case "user":       return "UserMessage.qml"
                case "permission": return "PermissionCard.qml"
                default:           return "AssistantMessage.qml"
            }
        }

        onLoaded: {
            if (!item) return
            item.messageType = delegateRoot.bindMessageType
            item.content     = delegateRoot.bindContent
            item.thinking    = delegateRoot.bindThinking
            item.streaming   = delegateRoot.bindStreaming
            delegateRoot.cachedParts = model.parts || []
            item.parts       = delegateRoot.cachedParts
        }
    }

    // Re-sync scalar properties
    onBindMessageTypeChanged: if (loader.item) loader.item.messageType = bindMessageType
    onBindContentChanged:     if (loader.item) loader.item.content = bindContent
    onBindThinkingChanged:    if (loader.item) loader.item.thinking = bindThinking
    onBindStreamingChanged:   if (loader.item) loader.item.streaming = bindStreaming

    // When partsVersion changes, re-read parts from model but only hand a
    // new array to the item when the composition changed — identical arrays
    // (same PartObject instances in the same order) are dropped so the
    // Repeater keeps its delegates alive across streaming updates.
    onBindPartsVersionChanged: {
        if (!loader.item || bindPartsVersion <= 0) return
        var fresh = model.parts || []
        if (sameParts(delegateRoot.cachedParts, fresh)) return
        delegateRoot.cachedParts = fresh
        loader.item.parts = fresh
    }
}
