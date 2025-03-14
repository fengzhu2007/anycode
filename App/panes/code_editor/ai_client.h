#ifndef AI_CLIENT_H
#define AI_CLIENT_H
#include "global.h"
#include <QObject>
#include <QTextCursor>
#include <utils/textutils.h>
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
    Q_OBJECT
public:
    explicit AIClient(QObject* parent=nullptr);
    ~AIClient();

    void openDocument(TextEditor::TextDocument* document);
    void request(CodeEditorView* editor,int timeout=0);
    void scheduleRequest(CodeEditorView *editor);
    void requestCompletions(CodeEditorView *editor);
    void handleCompletions(AIResponse *response,const Utils::Text::Position& position,CodeEditorView *editor);
    void cancelRunningRequest(CodeEditorView *editor);

    bool enabled();


public slots:
    void onRequestFinish();

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
