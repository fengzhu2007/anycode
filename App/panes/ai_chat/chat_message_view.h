#ifndef CHAT_MESSAGE_VIEW_H
#define CHAT_MESSAGE_VIEW_H

/**
 * @file chat_message_view.h
 * @brief 聊天气泡控件实现切换开关。
 *
 * ADY_USE_CHAT_MESSAGE_BUBBLE = 1(默认):使用新的 ChatMessageBubble
 *   —— 基于 QTextDocument/QTextBrowser,支持完整 markdown 渲染;
 * ADY_USE_CHAT_MESSAGE_BUBBLE = 0:回退到旧版 QLabel 实现的
 *   ChatMessageWidget(暂时保留,便于快速回滚对比)。
 *
 * 两个类的公开接口完全一致,所有调用点一律通过 ChatMessageView 别名
 * 引用,不要直接引用具体类名。
 */
#ifndef ADY_USE_CHAT_MESSAGE_BUBBLE
#define ADY_USE_CHAT_MESSAGE_BUBBLE 1
#endif

#if ADY_USE_CHAT_MESSAGE_BUBBLE
#include "chat_message_bubble.h"
namespace ady {
using ChatMessageView = ChatMessageBubble;
}
#else
#include "chat_message_widget.h"
namespace ady {
using ChatMessageView = ChatMessageWidget;
}
#endif

#endif // CHAT_MESSAGE_VIEW_H
