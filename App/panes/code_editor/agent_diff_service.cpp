#include "agent_diff_service.h"
#include "modules/options/options_settings.h"
#include "modules/options/agent_settings.h"
#include <QtConcurrent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <curl/curl.h>

namespace ady {

// libcurl write callback: accumulates response body into std::string
static size_t writeCallback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    auto *buf = static_cast<std::string*>(userdata);
    buf->append(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}

AgentDiffService::AgentDiffService(QObject *parent)
    : QObject(parent)
{
    m_watcher = new QFutureWatcher<DiffResult>(this);
    connect(m_watcher, &QFutureWatcher<DiffResult>::finished, this, [this]() {
        if (m_watcher->isCanceled()) {
            return;
        }
        DiffResult result = m_watcher->result();
        if (result.success) {
            emit diffReady(result.filePath, result.content);
        } else {
            emit diffFailed(result.filePath);
        }
    });
}

AgentDiffService::~AgentDiffService()
{
    if (m_watcher->isRunning()) {
        m_watcher->cancel();
        m_watcher->waitForFinished();
    }
}

void AgentDiffService::fetchDiff(const QString &filePath)
{
    // Cancel any in-flight request
    if (m_watcher->isRunning()) {
        m_watcher->cancel();
        m_watcher->waitForFinished();
    }

    int port = OptionsSettings::getInstance()->agentSettings().m_port;
    auto future = QtConcurrent::run(&AgentDiffService::fetchDiffSync, filePath, port);
    m_watcher->setFuture(future);
}

DiffResult AgentDiffService::fetchDiffSync(const QString &filePath, int port)
{
    DiffResult result;
    result.filePath = filePath;

    // URL-encode the file path for query parameter
    QString encodedPath = QUrl::toPercentEncoding(filePath);
    QString url = QString("http://127.0.0.1:%1/file/diff?file=%2")
                      .arg(port)
                      .arg(encodedPath);

    CURL *curl = curl_easy_init();
    if (!curl) {
        result.error = "Failed to init curl";
        return result;
    }

    QByteArray urlBytes = url.toUtf8();
    std::string responseBody;

    curl_easy_setopt(curl, CURLOPT_URL, urlBytes.constData());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        result.error = QString("curl error: %1").arg(curl_easy_strerror(res));
        return result;
    }

    result.content = parseResponse(QString::fromStdString(responseBody));
    result.success = true;
    return result;
}

cvs::DiffContent AgentDiffService::parseResponse(const QString &jsonBody)
{
    cvs::DiffContent content;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonBody.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return content;
    }

    // Response is a JSON array: [{file, status, hunks: [...], absolutePath}]
    QJsonArray arr = doc.array();
    if (arr.isEmpty()) {
        return content;
    }

    QJsonObject obj = arr.at(0).toObject();
    QString status = obj["status"].toString();

    // No changes or not tracked — return empty content
    if (status == "no_changes" || status == "no_snapshot") {
        return content;
    }

    QJsonArray hunks = obj["hunks"].toArray();
    for (const auto &hunkVal : hunks) {
        QJsonObject hunkObj = hunkVal.toObject();
        cvs::DiffHunk hunk;
        hunk.setOldStart(hunkObj["oldStart"].toInt());
        hunk.setOldCount(hunkObj["oldCount"].toInt());
        hunk.setNewStart(hunkObj["newStart"].toInt());
        hunk.setNewCount(hunkObj["newCount"].toInt());
        hunk.setHeader(hunkObj["header"].toString());

        QJsonArray lines = hunkObj["lines"].toArray();
        for (const auto &lineVal : lines) {
            QJsonObject lineObj = lineVal.toObject();
            QString type = lineObj["type"].toString();

            cvs::DiffLine::Type lineType = cvs::DiffLine::Context;
            if (type == "addition")
                lineType = cvs::DiffLine::Addition;
            else if (type == "deletion")
                lineType = cvs::DiffLine::Deletion;

            cvs::DiffLine line(lineType,
                               lineObj["content"].toString(),
                               lineObj["oldLineNo"].toInt(-1),
                               lineObj["newLineNo"].toInt(-1));
            hunk.addLine(line);
        }

        content.addHunk(hunk);
    }

    return content;
}

} // namespace ady
