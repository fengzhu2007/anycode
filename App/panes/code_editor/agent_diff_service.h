#ifndef AGENT_DIFF_SERVICE_H
#define AGENT_DIFF_SERVICE_H

#include "global.h"
#include "cvs/diff_content.h"
#include <QObject>
#include <QString>
#include <QFutureWatcher>

namespace ady {

struct DiffResult {
    QString filePath;
    cvs::DiffContent content;
    bool success = false;
    QString error;
};

class ANYENGINE_EXPORT AgentDiffService : public QObject
{
    Q_OBJECT
public:
    explicit AgentDiffService(QObject *parent = nullptr);
    ~AgentDiffService();

    void fetchDiff(const QString &filePath);

signals:
    void diffReady(const QString &filePath, const cvs::DiffContent &content);
    void diffFailed(const QString &filePath);

private:
    static DiffResult fetchDiffSync(const QString &filePath, int port);
    static cvs::DiffContent parseResponse(const QString &jsonBody);

    QFutureWatcher<DiffResult> *m_watcher = nullptr;
};

} // namespace ady

#endif // AGENT_DIFF_SERVICE_H
