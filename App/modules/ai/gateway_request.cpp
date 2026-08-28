#include "gateway_request.h"
#include "network/http/http_response.h"
#include "gateway_response.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QtConcurrent>
#include <QThread>
#include <QDeadlineTimer>
#include <QDebug>
#include <QUuid>
#include <QMutex>
#include <QMetaObject>

static char prefixKey[] = "prefix";
static char suffixKey[] = "suffix";
static char middleKey[] = "middle";

namespace ady{

// ========== SSE streaming context ==========
// Passed as CURLOPT_WRITEDATA, accumulates SSE data and emits chunks in real-time.
// The write callback runs in the worker thread (same thread as curl_easy_perform).
struct SseContext {
    QByteArray buffer;       // accumulates partial SSE data
    QString fullContent;     // accumulated text content
    GatewayRequest *request; // for emitting signals (queued connection across threads)
    volatile bool *abortFlag; // checked each callback invocation
};

// libcurl write callback — called each time a chunk arrives from the SSE stream
static size_t sseWriteCallback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t realsize = size * nmemb;
    auto *ctx = static_cast<SseContext*>(userdata);

    // Check abort flag — return 0 to cancel curl transfer immediately
    if (ctx->abortFlag && *ctx->abortFlag) return 0;

    ctx->buffer.append(static_cast<const char*>(ptr), realsize);

    // Process complete SSE lines
    while (true) {
        int idx = ctx->buffer.indexOf('\n');
        if (idx < 0) break;

        QByteArray line = ctx->buffer.left(idx).trimmed();
        ctx->buffer = ctx->buffer.mid(idx + 1);

        if (!line.startsWith("data: ")) continue;

        QByteArray rawData = line.mid(6);
        if (rawData == "[DONE]") {
            qDebug() << "[GatewayRequest] SSE [DONE] received, total=" << ctx->fullContent.size() << "chars";
            continue;
        }
        if (rawData.isEmpty()) continue;

        // Emit raw SSE data as-is — no format parsing
        QString content = QString::fromUtf8(rawData);
        ctx->fullContent += content;

        qDebug() << "[GatewayRequest] SSE chunk:" << rawData.size() << "bytes |" 
                 << rawData.left(120) << "| total=" << ctx->fullContent.size();

        if (ctx->request) {
            QMetaObject::invokeMethod(ctx->request, "chunkReceived",
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, content));
        }
    }
    return realsize;
}

// Progress callback — fires periodically even when no data arrives,
// ensures curl_easy_perform() returns promptly when abort is requested.
static int sseProgressCallback(void *clientp, curl_off_t, curl_off_t, curl_off_t, curl_off_t)
{
    volatile bool *abortFlag = static_cast<volatile bool*>(clientp);
    return *abortFlag ? 1 : 0;  // non-zero = abort curl transfer
}

// ========== GatewayWorker ==========

class GatewayWorker : public QObject
{
    Q_OBJECT
public:
    GatewayWorker(GatewayRequest* req, const QString& url, const QJsonObject& data)
        : QObject(nullptr), m_req(req), m_url(url), m_data(data) {}

public slots:
    void doWork() {
        auto response = new GatewayResponse();

        // Use libcurl directly (bypass HttpClient::post()) to install SSE callback
        // before curl_easy_perform(). HttpClient::post() sets the default body
        // callback which would overwrite any SSE callback we set afterwards.
        CURL *curl = m_req->curlHandle();

        QByteArray urlBytes = m_url.toUtf8();
        curl_easy_setopt(curl, CURLOPT_URL, urlBytes.constData());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        QByteArray bodyBytes = QJsonDocument(m_data).toJson(QJsonDocument::Compact);
        std::string bodyStr = bodyBytes.toStdString();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodyStr.c_str());

        // HTTP headers
        struct curl_slist *chunk = nullptr;
        chunk = curl_slist_append(chunk, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        // SSE streaming callback
        SseContext ctx;
        ctx.request = m_req;
        ctx.abortFlag = &m_req->m_abort;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, sseWriteCallback);

        // Progress callback: fires ~1x/sec, wakes up curl so abort is detected
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, sseProgressCallback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &m_req->m_abort);

        // Low-speed timeout: if no data arrives for 5 seconds, curl returns automatically.
        // This ensures the worker thread exits promptly when gateway stops or network drops.
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);   // 1 byte/sec threshold
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 5L);    // 5 seconds without data → abort

        // Execute — blocks until SSE stream ends, callback fires for each chunk
        CURLcode res = curl_easy_perform(curl);
        response->errorCode = (int)res;
        if (res != CURLE_OK) {
            response->errorMsg = curl_easy_strerror(res);
        }

        // Copy accumulated content into response body for backward compatibility
        response->body = ctx.fullContent;

        // Cleanup
        if (chunk) curl_slist_free_all(chunk);

        // If aborted (window closing), don't emit result — just clean up
        if (m_req->m_abort) {
            delete response;
            return;
        }

        emit resultReady(response);
    }
signals:
    void resultReady(GatewayResponse* response);

private:
    GatewayRequest* m_req;
    QJsonObject m_data;
    QString m_url;
};

// ========== GatewayRequest ==========

GatewayRequest::GatewayRequest(QObject* parent, const QJsonObject& data)
    : QObject(parent)
    , HttpClient()
    , m_data(data)
{
    this->setOption(CURLOPT_TIMEOUT, 30); // 30min timeout for local gateway

    // Build chat completions request body
    QJsonObject requestBody = {
        {"model", this->model()},
        {"messages", QJsonArray{
            QJsonObject{
                {"role", "user"},
                {"content", this->prompt() + "\n\nFill in the middle. Return only the completed code. Do not include any explanation, markdown, or code fences."}
            }
        }},
        {"stream", true},
        {"session_id", QUuid::createUuid().toString(QUuid::WithoutBraces)}
    };

    const QString url = "http://127.0.0.1:3456/v1/chat/completions";

    GatewayWorker *worker = new GatewayWorker(this, url, requestBody);
    worker->moveToThread(&workerThread);

    connect(&workerThread, &QThread::started, worker, &GatewayWorker::doWork);
    connect(&workerThread, &QThread::finished, &workerThread, &QThread::deleteLater);
    connect(worker, &GatewayWorker::resultReady, this, &GatewayRequest::handleResults);
}

GatewayRequest::~GatewayRequest(){
    qDebug("~GatewayRequest");
    m_abort = true;          // signal curl callbacks to cancel transfer
    workerThread.quit();
    // Timed wait: curl will exit within LOW_SPEED_TIME (5s) if no data arrives.
    // The abort flag + progress callback provide faster cancellation when data is flowing.
    if (!workerThread.wait(QDeadlineTimer(8000))) {
        qWarning("~GatewayRequest: worker thread did not exit in 8s, force terminating");
        workerThread.terminate();
        workerThread.wait();
    }
}

void GatewayRequest::call(){
    workerThread.start();
}

void GatewayRequest::handleResults(GatewayResponse* response){
    auto sender = static_cast<GatewayWorker*>(this->sender());
    delete sender;
    if(this->m_func){
        this->m_func(response);
    }
    emit this->finish();
}

QString GatewayRequest::model(){
    return QString("browser-llm");
}

QString GatewayRequest::prompt(){
    return QString::fromUtf8("<|fim_prefix|>%1<|fim_suffix|>%2<|fim_middle|>%3")
        .arg(m_data.find(prefixKey)->toString())
        .arg(m_data.find(suffixKey)->toString())
        .arg(m_data.find(middleKey)->toString());
}

NetworkResponse* GatewayRequest::customeAccess(const QString& name, QMap<QString,QVariant> data){
    return nullptr;
}

}
#include "gateway_request.moc"
