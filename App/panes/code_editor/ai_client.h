#ifndef AI_CLIENT_H
#define AI_CLIENT_H
#include "global.h"
#include <QObject>
#include "modules/ai/ai_request.h"
namespace TextEditor{
class TextDocument;
}

namespace ady{
class CodeEditorView;
class AIClientPrivate;
class AIResponse;
class ANYENGINE_EXPORT AIClient : public QObject
{
public:
    explicit AIClient(QObject* parent=nullptr);

    void openDocument(TextEditor::TextDocument* document);
    void scheduleRequest(CodeEditorView *editor);
    void requestCompletions(CodeEditorView *editor);
    void handleCompletions(AIResponse *response,CodeEditorView *editor);
    void cancelRunningRequest(CodeEditorView *editor);

    bool enabled();
private:
    struct ScheduleData
    {
        int cursorPosition = -1;
        QTimer *timer = nullptr;
    };

    QHash<CodeEditorView*, AiRequest*> m_runningRequests;
    QHash<CodeEditorView*, ScheduleData> m_scheduledRequests;

    AIClientPrivate* d;
};

}
#endif // AI_CLIENT_H
