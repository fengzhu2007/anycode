#include "chat_service.h"
#include <QJsonDocument>
#include <QDateTime>
#include <QDebug>
#include <QtConcurrent>
#include <QMetaType>

namespace ady{

// ---- curl static callbacks ----

size_t ChatService::writeCallback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    QByteArray *buf = static_cast<QByteArray*>(userdata);
    size_t total = size * nmemb;
    buf->append(static_cast<char*>(ptr), total);
    return total;
}

size_t ChatService::eventStreamCallback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    ChatService *svc = static_cast<ChatService*>(userdata);
    size_t total = size * nmemb;
    QByteArray chunk(static_cast<char*>(ptr), total);
    svc->processEventStream(chunk);
    return total;
}

int ChatService::progressCallback(void *clientp, curl_off_t /*dltotal*/, curl_off_t /*dlnow*/,
                                  curl_off_t /*ultotal*/, curl_off_t /*ulnow*/)
{
    volatile bool *abortFlag = static_cast<volatile bool*>(clientp);
    return *abortFlag ? 1 : 0;
}

// ---- construction / destruction ----

ChatService::ChatService(QObject *parent)
    : QObject(parent)
    , m_baseUrl("http://127.0.0.1:4096")
    , m_requesting(false)
    , m_abort(false)
    , m_eventCurl(nullptr)
    , m_eventAbort(false)
{
    // 注册自定义类型用于跨线程信号传递
    qRegisterMetaType<OpenCodeSession>("OpenCodeSession");
    qRegisterMetaType<QList<OpenCodeSession>>("QList<OpenCodeSession>");
    qRegisterMetaType<OpenCodeModel>("OpenCodeModel");
    qRegisterMetaType<QList<OpenCodeModel>>("QList<OpenCodeModel>");

    // 启动时连接事件流
    connectEventStream();
}

ChatService::~ChatService()
{
    // 中止所有请求
    m_abort = true;
    m_eventAbort = true;

    // 等待所有线程结束
    if(m_future.isRunning()){
        m_future.waitForFinished();
    }
    if(m_eventFuture.isRunning()){
        m_eventFuture.waitForFinished();
    }
}

// ---- configuration ----

void ChatService::setBaseUrl(const QString &url)
{
    m_baseUrl = url;
    if(m_baseUrl.endsWith('/')){
        m_baseUrl.chop(1);
    }
    // 重新连接事件流
    disconnectEventStream();
    connectEventStream();
}

// ---- helper: build curl handle ----

CURL* ChatService::createCurlHandle()
{
    CURL *curl = curl_easy_init();
    if(!curl) return nullptr;

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    return curl;
}

// ---- list sessions: GET /session ----

void ChatService::listSessions()
{
    qDebug() << "[ChatService] listSessions called, m_requesting=" << m_requesting;
    if(m_requesting) return;
    m_requesting = true;
    m_abort = false;

    QString url = m_baseUrl + "/session";
    qDebug() << "[ChatService] listSessions requesting:" << url;

    m_future = QtConcurrent::run([this, url](){
        CURL *curl = createCurlHandle();
        if(!curl){
            emit sessionsReceived({}, tr("Failed to initialize network"));
            m_requesting = false;
            return;
        }

        QByteArray responseData;
        curl_easy_setopt(curl, CURLOPT_URL, url.toUtf8().constData());
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if(res != CURLE_OK){
            emit sessionsReceived({}, QString::fromUtf8(curl_easy_strerror(res)));
            m_requesting = false;
            return;
        }

        // parse response: array of sessions
        qDebug() << "[ChatService] listSessions response:" << responseData;
        QList<OpenCodeSession> sessions;
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        QJsonArray arr = doc.array();
        for(const auto &val : arr){
            QJsonObject obj = val.toObject();
            OpenCodeSession s;
            s.id = obj["id"].toString();
            s.title = obj["title"].toString();
            s.agent = obj["agent"].toString();
            s.timeCreated = obj["timeCreated"].toVariant().toLongLong();
            s.timeUpdated = obj["timeUpdated"].toVariant().toLongLong();
            if(!s.id.isEmpty()){
                sessions.append(s);
            }
        }
        m_sessions = sessions;
        emit sessionsReceived(sessions, {});
        m_requesting = false;
    });
}

// ---- create session: POST /session ----

void ChatService::createSession(const QString &title)
{
    if(m_requesting) return;
    m_requesting = true;
    m_abort = false;

    QString url = m_baseUrl + "/session";

    QJsonObject body;
    if(!title.isEmpty()){
        body["title"] = title;
    }
    // model 字段是对象，包含 id 和 providerID
    if(!m_providerID.isEmpty() && !m_modelID.isEmpty()){
        QJsonObject model;
        model["id"] = m_modelID;           // 使用 id 而不是 modelID
        model["providerID"] = m_providerID;
        body["model"] = model;
    }

    QByteArray postData = QJsonDocument(body).toJson(QJsonDocument::Compact);
    qDebug() << "[ChatService] createSession request:" << postData;

    m_future = QtConcurrent::run([this, url, postData](){
        CURL *curl = createCurlHandle();
        if(!curl){
            emit sessionCreated({}, tr("Failed to initialize network"));
            m_requesting = false;
            return;
        }

        QByteArray responseData;
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, url.toUtf8().constData());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.constData());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postData.size());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if(res != CURLE_OK){
            emit sessionCreated({}, QString::fromUtf8(curl_easy_strerror(res)));
            m_requesting = false;
            return;
        }

        // parse response: created session
        qDebug() << "[ChatService] createSession response:" << responseData;
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        QJsonObject obj = doc.object();
        OpenCodeSession s;
        s.id = obj["id"].toString();
        s.title = obj["title"].toString();
        s.agent = obj["agent"].toString();
        s.timeCreated = obj["timeCreated"].toVariant().toLongLong();
        s.timeUpdated = obj["timeUpdated"].toVariant().toLongLong();

        if(s.id.isEmpty()){
            emit sessionCreated({}, tr("Failed to create session"));
        }else{
            m_sessions.prepend(s);
            m_currentSessionId = s.id;
            emit sessionCreated(s, {});
        }
        m_requesting = false;
    });
}

// ---- delete session: DELETE /session/{id} ----

void ChatService::deleteSession(const QString &sessionId)
{
    if(m_requesting) return;
    m_requesting = true;
    m_abort = false;

    QString url = m_baseUrl + "/session/" + sessionId;

    m_future = QtConcurrent::run([this, url, sessionId](){
        CURL *curl = createCurlHandle();
        if(!curl){
            emit sessionDeleted(sessionId, tr("Failed to initialize network"));
            m_requesting = false;
            return;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.toUtf8().constData());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if(res != CURLE_OK){
            emit sessionDeleted(sessionId, QString::fromUtf8(curl_easy_strerror(res)));
        }else{
            // remove from local cache
            for(int i = 0; i < m_sessions.size(); ++i){
                if(m_sessions[i].id == sessionId){
                    m_sessions.removeAt(i);
                    break;
                }
            }
            if(m_currentSessionId == sessionId){
                m_currentSessionId.clear();
            }
            emit sessionDeleted(sessionId, {});
        }
        m_requesting = false;
    });
}

// ---- list models: GET /api/model ----

void ChatService::listModels()
{
    qDebug() << "[ChatService] listModels called, m_requesting=" << m_requesting;
    if(m_requesting) return;
    m_requesting = true;
    m_abort = false;

    QString url = m_baseUrl + "/api/model";
    qDebug() << "[ChatService] listModels requesting:" << url;

    m_future = QtConcurrent::run([this, url](){
        CURL *curl = createCurlHandle();
        if(!curl){
            emit modelsReceived({}, tr("Failed to initialize network"));
            m_requesting = false;
            return;
        }

        QByteArray responseData;
        curl_easy_setopt(curl, CURLOPT_URL, url.toUtf8().constData());
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if(res != CURLE_OK){
            emit modelsReceived({}, QString::fromUtf8(curl_easy_strerror(res)));
            m_requesting = false;
            return;
        }

        // parse response: { location: {...}, data: [...] }
        qDebug() << "[ChatService] listModels response:" << responseData;
        QList<OpenCodeModel> models;
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        QJsonObject root = doc.object();
        QJsonArray data = root["data"].toArray();

        for(const auto &val : data){
            QJsonObject obj = val.toObject();

            // 只解析启用的模型
            if(!obj["enabled"].toBool()) continue;

            // 只解析支持文本输出的模型
            QJsonObject caps = obj["capabilities"].toObject();
            QJsonArray outputs = caps["output"].toArray();
            bool supportsText = false;
            for(const auto &o : outputs){
                if(o.toString() == "text"){
                    supportsText = true;
                    break;
                }
            }
            if(!supportsText) continue;

            OpenCodeModel m;
            m.providerID = obj["providerID"].toString();
            m.modelID = obj["id"].toString();  // 使用 id 字段
            m.name = obj["name"].toString();
            if(m.name.isEmpty()){
                m.name = m.modelID;
            }
            if(!m.providerID.isEmpty() && !m.modelID.isEmpty()){
                models.append(m);
            }
        }
        qDebug() << "[ChatService] parsed" << models.size() << "text models from" << data.size() << "total";
        m_models = models;
        emit modelsReceived(models, {});
        m_requesting = false;
    });
}

// ---- send message: POST /session/{id}/message ----

void ChatService::sendMessage(const QString &sessionId, const QString &content)
{
    if(m_requesting) return;
    m_requesting = true;
    m_abort = false;

    QString url = m_baseUrl + "/session/" + sessionId + "/message";

    // build request body
    QJsonObject textPart;
    textPart["type"] = "text";
    textPart["text"] = content;

    QJsonArray parts;
    parts.append(textPart);

    QJsonObject body;
    body["parts"] = parts;

    // optional model
    if(!m_providerID.isEmpty() && !m_modelID.isEmpty()){
        QJsonObject model;
        model["providerID"] = m_providerID;
        model["modelID"] = m_modelID;
        body["model"] = model;
    }

    QByteArray postData = QJsonDocument(body).toJson(QJsonDocument::Compact);

    emit streamStarted(sessionId);

    m_future = QtConcurrent::run([this, url, postData, sessionId](){
        CURL *curl = curl_easy_init();
        if(!curl){
            emit streamFinished(sessionId, tr("Failed to initialize network"));
            m_requesting = false;
            return;
        }

        // 发送消息后不等待服务器完整响应，响应通过 SSE 事件流接收
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);     // 连接 + 发送超时
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

        QByteArray responseData;
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, url.toUtf8().constData());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.constData());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postData.size());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        // 无论成功还是超时，都认为消息已发送（响应通过 SSE 接收）
        // 不发射 streamFinished，由 SSE 事件 session.status=idle 触发
        qDebug() << "[ChatService] sendMessage result:" << curl_easy_strerror(res) 
                 << "response:" << responseData;
        
        if(res == CURLE_OK){
            // 正常响应，检查是否有错误
            QJsonDocument doc = QJsonDocument::fromJson(responseData);
            QJsonObject obj = doc.object();
            if(obj.contains("error")){
                QString err = obj["error"].toObject()["message"].toString();
                // 只有真正的错误才报错
                emit streamFinished(sessionId, err.isEmpty() ? tr("Unknown error") : err);
            }
            // 正常响应不发射 streamFinished，等待 SSE 事件
        }else if(res == CURLE_OPERATION_TIMEDOUT){
            // 超时但消息已发送，不报错（响应通过 SSE 接收）
            qDebug() << "[ChatService] sendMessage timeout, waiting for SSE events";
            // 不发射 streamFinished，等待 SSE 事件 session.status=idle
        }else{
            // 其他错误（连接失败等）
            emit streamFinished(sessionId, QString::fromUtf8(curl_easy_strerror(res)));
        }
        m_requesting = false;
    });
}

// ---- abort session: POST /session/{id}/abort ----

void ChatService::abortSession(const QString &sessionId)
{
    QString url = m_baseUrl + "/session/" + sessionId + "/abort";

    QtConcurrent::run([url](){
        CURL *curl = curl_easy_init();
        if(!curl) return;

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_URL, url.toUtf8().constData());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    });
}

// ---- SSE event stream: GET /global/event ----

void ChatService::connectEventStream()
{
    if(m_eventCurl) return;

    m_eventAbort = false;
    QString url = m_baseUrl + "/global/event";

    m_eventFuture = QtConcurrent::run([this, url](){
        m_eventCurl = curl_easy_init();
        if(!m_eventCurl){
            qWarning() << "Failed to create event stream curl handle";
            return;
        }

        curl_easy_setopt(m_eventCurl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(m_eventCurl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(m_eventCurl, CURLOPT_URL, url.toUtf8().constData());
        curl_easy_setopt(m_eventCurl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(m_eventCurl, CURLOPT_WRITEFUNCTION, eventStreamCallback);
        curl_easy_setopt(m_eventCurl, CURLOPT_WRITEDATA, this);

        // 长连接，不设置超时
        curl_easy_setopt(m_eventCurl, CURLOPT_CONNECTTIMEOUT, 10L);

        // 启用进度回调用于中止
        curl_easy_setopt(m_eventCurl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(m_eventCurl, CURLOPT_XFERINFOFUNCTION, progressCallback);
        curl_easy_setopt(m_eventCurl, CURLOPT_XFERINFODATA, &m_eventAbort);

        CURLcode res = curl_easy_perform(m_eventCurl);
        if(res != CURLE_OK && res != CURLE_ABORTED_BY_CALLBACK){
            qWarning() << "Event stream error:" << curl_easy_strerror(res);
        }

        curl_easy_cleanup(m_eventCurl);
        m_eventCurl = nullptr;
    });
}

void ChatService::disconnectEventStream()
{
    m_eventAbort = true;
    // 等待事件流线程结束
    if(m_eventFuture.isRunning()){
        m_eventFuture.waitForFinished();
    }
}

// ---- process SSE events ----

void ChatService::processEventStream(const QByteArray &chunk)
{
    m_sseBuffer.append(chunk);

    // SSE 格式: data: {...}\n\n
    while(true){
        int idx = m_sseBuffer.indexOf("\n\n");
        if(idx < 0){
            // 尝试 \r\n\r\n
            idx = m_sseBuffer.indexOf("\r\n\r\n");
            if(idx < 0) break;
        }

        QString event = m_sseBuffer.left(idx).trimmed();
        int endPos = m_sseBuffer.indexOf("\n\n", idx);
        if(endPos < 0){
            endPos = m_sseBuffer.indexOf("\r\n\r\n", idx);
            m_sseBuffer = m_sseBuffer.mid(endPos + 4);
        }else{
            m_sseBuffer = m_sseBuffer.mid(idx + 2);
        }

        // 解析 SSE 事件
        if(event.startsWith("data:")){
            QString jsonStr = event.mid(5).trimmed();
            if(jsonStr.isEmpty()) continue;

            qDebug() << "[ChatService] SSE event:" << jsonStr;

            QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
            QJsonObject obj = doc.object();
            
            // 事件结构: { payload: { type: "...", properties: {...} } }
            QJsonObject payload = obj["payload"].toObject();
            QString type = payload["type"].toString();

            if(type == "message.part.delta"){
                // 流式增量输出
                QJsonObject props = payload["properties"].toObject();
                QString sessionId = props["sessionID"].toString();
                QString delta = props["delta"].toString();
                if(!delta.isEmpty()){
                    emit streamChunk(sessionId, delta);
                }
            }else if(type == "message.part.updated"){
                // 完整更新（可用于获取完整文本）
                QJsonObject props = payload["properties"].toObject();
                QString sessionId = props["sessionID"].toString();
                QJsonObject part = props["part"].toObject();
                QString partType = part["type"].toString();
                if(partType == "text"){
                    // 完整文本，可用于保存历史
                }
            }else if(type == "session.status"){
                QJsonObject props = payload["properties"].toObject();
                QString sessionId = props["sessionID"].toString();
                QJsonObject status = props["status"].toObject();
                QString statusType = status["type"].toString();
                emit sessionStatusChanged(sessionId, statusType);

                if(statusType == "idle"){
                    emit streamFinished(sessionId, {});
                }
            }else if(type == "session.updated"){
                // 会话更新事件，表示 AI 响应完成
                QJsonObject props = payload["properties"].toObject();
                QString sessionId = props["sessionID"].toString();
                qDebug() << "[ChatService] session.updated for session:" << sessionId;
                emit streamFinished(sessionId, {});
            }else if(type == "message.updated"){
                // 消息完整更新
                QJsonObject props = payload["properties"].toObject();
                QString sessionId = props["sessionID"].toString();
            }
        }
    }
}

}
