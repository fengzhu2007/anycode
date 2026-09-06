#include "chat_service.h"
#include <QJsonDocument>
#include <QDateTime>
#include <QDebug>
#include <QtConcurrent>
#include <QMetaType>
#include <QMetaObject>
#include <QUrl>

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
    qRegisterMetaType<OpenCodeSession>("OpenCodeSession");
    qRegisterMetaType<QList<OpenCodeSession>>("QList<OpenCodeSession>");
    qRegisterMetaType<OpenCodeModel>("OpenCodeModel");
    qRegisterMetaType<QList<OpenCodeModel>>("QList<OpenCodeModel>");
    qRegisterMetaType<OpenCodeMessage>("OpenCodeMessage");
    qRegisterMetaType<QList<OpenCodeMessage>>("QList<OpenCodeMessage>");

    // Reconnect timer for event stream
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, [this](){ connectEventStream(); });

    // Internal signal: event stream thread ended → reconnect logic
    connect(this, &ChatService::eventStreamEnded, this, &ChatService::onEventStreamEnded);

    connectEventStream();
}

ChatService::~ChatService()
{
    m_abort = true;
    m_eventAbort = true;

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

    QString url = m_baseUrl + "/experimental/session";
    //qDebug() << "[ChatService] listSessions requesting:" << url;

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
        //qDebug() << "[ChatService] listSessions response:" << responseData;
        QList<OpenCodeSession> sessions;
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        //qDebug() <<"[ChatService] listSessions response:"<< doc.toJson();
        QJsonArray arr = doc.array();
        for(const auto &val : arr){
            QJsonObject obj = val.toObject();
            OpenCodeSession s;
            s.id = obj["id"].toString();
            s.title = obj["title"].toString();
            s.agent = obj["agent"].toString();
            //qDebug()<<"session:"<<s.id<<s.title<<s.agent;
            QJsonObject timeObj = obj["time"].toObject();
            s.timeCreated = timeObj["created"].toVariant().toLongLong();
            s.timeUpdated = timeObj["updated"].toVariant().toLongLong();
            QJsonObject modelObj = obj["model"].toObject();
            if(!modelObj.isEmpty()){
                s.modelProviderID = modelObj["providerID"].toString();
                s.modelID = modelObj["id"].toString();
            }
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

void ChatService::createSession(const QString &title, const QString &directory, const QString &providerID, const QString &modelID)
{
    if(m_requesting) return;
    m_requesting = true;
    m_abort = false;

    QString url = m_baseUrl + "/session";
    if(!directory.isEmpty()){
        url += "?directory=" + QUrl::toPercentEncoding(directory);
    }

    QJsonObject body;
    if(!title.isEmpty()){
        body["title"] = title;
    }
    // use passed model, fallback to global model
    QString pid = providerID.isEmpty() ? m_providerID : providerID;
    QString mid = modelID.isEmpty() ? m_modelID : modelID;
    if(!pid.isEmpty() && !mid.isEmpty()){
        QJsonObject model;
        model["id"] = mid;
        model["providerID"] = pid;
        body["model"] = model;
    }

    QByteArray postData = QJsonDocument(body).toJson(QJsonDocument::Compact);
    //qDebug() << "[ChatService] createSession request:" << postData;

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
        QJsonObject timeObj = obj["time"].toObject();
        s.timeCreated = timeObj["created"].toVariant().toLongLong();
        s.timeUpdated = timeObj["updated"].toVariant().toLongLong();
        QJsonObject modelObj = obj["model"].toObject();
        if(!modelObj.isEmpty()){
            s.modelProviderID = modelObj["providerID"].toString();
            s.modelID = modelObj["id"].toString();
        }

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

    qDebug()<<"[deleteSession]11111111111111111111111111111111111 deleteSession";
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
    //qDebug() << "[ChatService] listModels called, m_requesting=" << m_requesting;
    if(m_requesting) return;
    m_requesting = true;
    m_abort = false;

    QString url = m_baseUrl + "/api/model";
    //qDebug() << "[ChatService] listModels requesting:" << url;

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
       // qDebug() << "[ChatService] listModels response:" << responseData;
        QList<OpenCodeModel> models;
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        QJsonObject root = doc.object();
        QJsonArray data = root["data"].toArray();

        for(const auto &val : data){
            QJsonObject obj = val.toObject();
            QString modelId = obj["id"].toString();
            QString providerID = obj["providerID"].toString();
            bool enabled = obj["enabled"].toBool();

            /*qDebug() << "[ChatService] model:" << modelId
                     << "provider:" << providerID
                     << "enabled:" << enabled
                     << "raw:" << QJsonDocument(obj).toJson(QJsonDocument::Compact);*/

            if(!enabled) {
                qDebug() << "[ChatService] SKIP (not enabled):" << modelId;
                continue;
            }

            QJsonObject caps = obj["capabilities"].toObject();
            QJsonArray outputs = caps["output"].toArray();
            if(!outputs.isEmpty()) {
                bool supportsText = false;
                for(const auto &o : outputs){
                    if(o.toString() == "text"){
                        supportsText = true;
                        break;
                    }
                }
                if(!supportsText) {
                    qDebug() << "[ChatService] SKIP (no text output):" << modelId
                             << "outputs:" << outputs;
                    continue;
                }
            }

            OpenCodeModel m;
            m.providerID = obj["providerID"].toString();
            m.modelID = obj["id"].toString();
            m.name = obj["name"].toString();
            if(m.name.isEmpty()){
                m.name = m.modelID;
            }
            if(!m.providerID.isEmpty() && !m.modelID.isEmpty()){
                models.append(m);
            }
        }
        //qDebug() << "[ChatService] parsed" << models.size() << "text models from" << data.size() << "total";
        m_models = models;
        emit modelsReceived(models, {});
        m_requesting = false;
    });
}

// ---- send message: POST /session/{id}/message ----

bool ChatService::sendMessage(const QString &sessionId, const QString &content, const QString &providerID, const QString &modelID)
{
    if(m_requesting) {
        qDebug() << "[ChatService] sendMessage SKIPPED: m_requesting=true";
        return false;
    }
    m_requesting = true;
    m_abort = false;

    m_sessionContentSizes[sessionId] += content.toUtf8().size();

    QString url = m_baseUrl + "/session/" + sessionId + "/message";

    // build request body
    QJsonObject textPart;
    textPart["type"] = "text";
    textPart["text"] = content;

    QJsonArray parts;
    parts.append(textPart);

    QJsonObject body;
    body["parts"] = parts;

    // use passed model, fallback to global model
    QString pid = providerID.isEmpty() ? m_providerID : providerID;
    QString mid = modelID.isEmpty() ? m_modelID : modelID;
    if(!pid.isEmpty() && !mid.isEmpty()){
        QJsonObject model;
        model["providerID"] = pid;
        model["modelID"] = mid;
        body["model"] = model;
    }

    qDebug() << "[ChatService] sendMessage" << sessionId
             << "model=" << pid + "/" + mid
             << "content=" << content;

    QByteArray postData = QJsonDocument(body).toJson(QJsonDocument::Compact);

    emit streamStarted(sessionId);

    m_future = QtConcurrent::run([this, url, postData, sessionId](){
        CURL *curl = curl_easy_init();
        if(!curl){
            emit streamFinished(sessionId, tr("Failed to initialize network"));
            m_requesting = false;
            return;
        }

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
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

        if(res == CURLE_OK){
            QJsonDocument doc = QJsonDocument::fromJson(responseData);
            QJsonObject obj = doc.object();
            if(obj.contains("error")){
                QString err = obj["error"].toObject()["message"].toString();
                emit streamFinished(sessionId, err.isEmpty() ? tr("Unknown error") : err);
            }
        }else if(res == CURLE_OPERATION_TIMEDOUT){
            // timeout is expected - response comes via SSE events
        }else{
            emit streamFinished(sessionId, QString::fromUtf8(curl_easy_strerror(res)));
        }
        m_requesting = false;
    });

    return true;
}

// ---- load session messages: GET /session/{id}/message ----

void ChatService::loadSessionMessages(const QString &sessionId, int limit)
{
    QString url = m_baseUrl + "/session/" + sessionId + "/message?limit=" + QString::number(limit);

    QtConcurrent::run([this, url, sessionId](){
        CURL *curl = curl_easy_init();
        if(!curl){
            emit messagesReceived(sessionId, {}, tr("Failed to init curl"));
            return;
        }

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_URL, url.toUtf8().constData());
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);

        QByteArray responseData;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if(res != CURLE_OK){
            emit messagesReceived(sessionId, {}, QString::fromUtf8(curl_easy_strerror(res)));
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        QJsonArray arr = doc.array();
        QList<OpenCodeMessage> messages;

        for(const auto &val : arr){
            QJsonObject msgObj = val.toObject();
            qDebug()<<"111111111[message object:]"<<msgObj;
            QJsonObject info = msgObj["info"].toObject();
            QJsonArray parts = msgObj["parts"].toArray();

            // Skip messages with errors (e.g., aborted attempts)
            if(!info["error"].toObject().isEmpty()){
                continue;
            }

            QString role = info["role"].toString();
            QString id = info["id"].toString();
            qint64 timeCreated = static_cast<qint64>(info["time"].toObject()["created"].toDouble());

            QString text;
            for(const auto &p : parts){
                QJsonObject partObj = p.toObject();
                QString type = partObj["type"].toString();

                if(type == "text"){
                    QString partText = partObj["text"].toString();
                    qDebug() << "[ChatService] text part length:" << partText.length()
                             << "contains ```:" << partText.contains("```")
                             << "preview:" << partText.left(200);
                    if(!text.isEmpty()) text += "\n";
                    text += partText;
                }else if(type == "reasoning"){
                    QString think = partObj["text"].toString().trimmed();
                    if(!think.isEmpty()){
                        if(!text.isEmpty()) text += "\n";
                        text += "<think>" + think + "</think>";
                    }
                }else if(type == "code"){
                    QString lang = partObj["language"].toString();
                    QString code = partObj["code"].toString();
                    if(code.isEmpty()) code = partObj["text"].toString();
                    if(!text.isEmpty()) text += "\n";
                    text += "```" + lang + "\n" + code + "\n```";
                }
                // step-start, step-finish, and other metadata types are silently skipped
            }

            if((role == "user" || role == "assistant") && !text.isEmpty()){
                OpenCodeMessage msg;
                msg.id = id;
                msg.role = role;
                msg.text = text;
                msg.timeCreated = timeCreated;
                messages.append(msg);
            }
        }

        qDebug() << "[ChatService] loaded" << messages.size() << "messages for session" << sessionId;
        emit messagesReceived(sessionId, messages, {});
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

// ---- update session title: PATCH /session/{id} ----

void ChatService::updateSessionTitle(const QString &sessionId, const QString &title)
{
    QString url = m_baseUrl + "/session/" + sessionId;

    QJsonObject body;
    body["title"] = title;
    QByteArray bodyBytes = QJsonDocument(body).toJson(QJsonDocument::Compact);

    QtConcurrent::run([url, bodyBytes](){
        CURL *curl = curl_easy_init();
        if(!curl) return;

        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_URL, url.toUtf8().constData());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodyBytes.constData());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)bodyBytes.size());

        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK){
            qWarning() << "[ChatService] Update title failed:" << curl_easy_strerror(res);
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    });
}

// ---- compact session: POST /api/session/{id}/compact ----

void ChatService::compactSession(const QString &sessionId)
{
    QString url = m_baseUrl + "/api/session/" + sessionId + "/compact";

    QtConcurrent::run([url, sessionId](){
        CURL *curl = curl_easy_init();
        if(!curl){
            qWarning() << "[ChatService] compactSession: failed to init curl";
            return;
        }

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_URL, url.toUtf8().constData());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);

        QByteArray responseData;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if(res != CURLE_OK){
            qWarning() << "[ChatService] compactSession failed:" << curl_easy_strerror(res);
        }else{
            qDebug() << "[ChatService] compactSession request sent for session:" << sessionId
                     << "response:" << responseData;
        }
    });
}

// ---- SSE event stream: GET /global/event ----

void ChatService::connectEventStream()
{
    if(m_eventCurl) return;

    m_eventAbort = false;
    QString url = m_baseUrl + "/global/event";

    m_eventFuture = QtConcurrent::run([this, url](){
        registerMcpServer();

        m_eventCurl = curl_easy_init();
        if(!m_eventCurl){
            qWarning() << "Failed to create event stream curl handle";
            QMetaObject::invokeMethod(this, "eventStreamEnded", Qt::QueuedConnection,
                                      Q_ARG(bool, false));
            return;
        }

        curl_easy_setopt(m_eventCurl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(m_eventCurl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(m_eventCurl, CURLOPT_URL, url.toUtf8().constData());
        curl_easy_setopt(m_eventCurl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(m_eventCurl, CURLOPT_WRITEFUNCTION, eventStreamCallback);
        curl_easy_setopt(m_eventCurl, CURLOPT_WRITEDATA, this);

        curl_easy_setopt(m_eventCurl, CURLOPT_CONNECTTIMEOUT, 10L);

        curl_easy_setopt(m_eventCurl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(m_eventCurl, CURLOPT_XFERINFOFUNCTION, progressCallback);
        curl_easy_setopt(m_eventCurl, CURLOPT_XFERINFODATA, &m_eventAbort);

        // Mark as connected before blocking call
        m_connected = true;
        m_reconnectRetry = 0;
        QMetaObject::invokeMethod(this, "connectionChanged", Qt::QueuedConnection,
                                  Q_ARG(bool, true));

        CURLcode res = curl_easy_perform(m_eventCurl);
        bool wasConnected = m_connected;
        m_connected = false;

        if(res != CURLE_OK && res != CURLE_ABORTED_BY_CALLBACK){
            qWarning() << "Event stream error:" << curl_easy_strerror(res);
        }

        QMetaObject::invokeMethod(this, "connectionChanged", Qt::QueuedConnection,
                                  Q_ARG(bool, false));

        curl_easy_cleanup(m_eventCurl);
        m_eventCurl = nullptr;

        // Notify main thread → triggers reconnect
        QMetaObject::invokeMethod(this, "eventStreamEnded", Qt::QueuedConnection,
                                  Q_ARG(bool, wasConnected));
    });
}

void ChatService::disconnectEventStream()
{
    m_eventAbort = true;
    if(m_reconnectTimer) m_reconnectTimer->stop();
    m_reconnectRetry = 0;
    if(m_eventFuture.isRunning()){
        m_eventFuture.waitForFinished();
    }
    if(m_connected){
        m_connected = false;
        emit connectionChanged(false);
    }
}

void ChatService::onEventStreamEnded(bool wasConnected)
{
    Q_UNUSED(wasConnected);
    // Auto-reconnect unless explicitly disconnected
    if(!m_reconnectTimer->isActive()){
        scheduleReconnect();
    }
}

void ChatService::scheduleReconnect()
{
    int delaySec = qMin(2 << m_reconnectRetry, 30);
    m_reconnectRetry++;
    qDebug() << "[ChatService] Reconnecting event stream in" << delaySec
             << "seconds (retry" << m_reconnectRetry << ")";
    m_reconnectTimer->start(delaySec * 1000);
}

// ---- MCP server registration: POST /mcp + POST /mcp/{name}/connect ----

void ChatService::registerMcpServer()
{
    if(m_mcpRegistered) return;

    QString baseUrl = m_baseUrl;
    uint16_t gwPort = m_gatewayPort;

    QtConcurrent::run([this, baseUrl, gwPort]() {
        QString addUrl = baseUrl + "/mcp";
        CURL *curl = createCurlHandle();
        if(!curl) return;

        QJsonObject configObj;
        configObj["type"] = "remote";
        configObj["url"] = QString("http://127.0.0.1:%1/mcp").arg(gwPort);

        QJsonObject body;
        body["name"] = "anycode";
        body["config"] = configObj;

        QByteArray bodyBytes = QJsonDocument(body).toJson(QJsonDocument::Compact);

        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        QByteArray responseData;
        curl_easy_setopt(curl, CURLOPT_URL, addUrl.toUtf8().constData());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodyBytes.constData());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)bodyBytes.size());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);

        if(res != CURLE_OK){
            qWarning() << "[ChatService] MCP add failed:" << curl_easy_strerror(res);
            curl_easy_cleanup(curl);
            return;
        }

        QJsonDocument respDoc = QJsonDocument::fromJson(responseData);
        if(respDoc.isNull() || !respDoc.isObject()){
            qDebug() << "[ChatService] MCP add response (raw):" << responseData;
        }

        qDebug() << "[ChatService] MCP server added: anycode ->" << configObj["url"].toString();
        curl_easy_cleanup(curl);

        QString connectUrl = baseUrl + "/mcp/anycode/connect";
        curl = createCurlHandle();
        if(!curl) return;

        responseData.clear();
        curl_easy_setopt(curl, CURLOPT_URL, connectUrl.toUtf8().constData());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

        res = curl_easy_perform(curl);
        if(res == CURLE_OK){
            m_mcpRegistered = true;
            qDebug() << "[ChatService] MCP server connected: anycode";
        } else {
            qWarning() << "[ChatService] MCP connect failed:" << curl_easy_strerror(res);
        }

        curl_easy_cleanup(curl);
    });
}

// ---- process SSE events ----

void ChatService::processEventStream(const QByteArray &chunk)
{
    m_sseBuffer.append(chunk);

    while(true){
        int idx = m_sseBuffer.indexOf("\n\n");
        if(idx < 0){
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

        //qDebug()<<event;
        if(event.startsWith("data:")){
            QString jsonStr = event.mid(5).trimmed();
            if(jsonStr.isEmpty()) continue;

            QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
            QJsonObject obj = doc.object();
            
            QJsonObject payload = obj["payload"].toObject();
            QString type = payload["type"].toString();

            if(type == "message.part.delta"){
                QJsonObject props = payload["properties"].toObject();
                QString sessionId = props["sessionID"].toString();
                QString delta = props["delta"].toString();
                if(!delta.isEmpty()){
                    m_sessionContentSizes[sessionId] += delta.toUtf8().size();
                    QMetaObject::invokeMethod(this, "streamChunk", Qt::QueuedConnection,
                                              Q_ARG(QString, sessionId), Q_ARG(QString, delta));
                }
            }else if(type == "message.part.updated"){
                QJsonObject props = payload["properties"].toObject();
                QString sessionId = props["sessionID"].toString();
                QJsonObject part = props["part"].toObject();
                QString partType = part["type"].toString();
                if(partType == "text"){
                    // text content handled via message.part.delta
                }else if(partType == "thinking"){
                    QString content = part["content"].toString();
                    if(!content.isEmpty()){
                        qDebug() << "[ChatService] thinking part received, length=" << content.length();
                        QMetaObject::invokeMethod(this, "streamThinking", Qt::QueuedConnection,
                                                  Q_ARG(QString, sessionId), Q_ARG(QString, content));
                    }
                }else if(partType == "tool"){
                    // OpenCode native tool format: lifecycle states pending → running → completed
                    QString toolName = part["tool"].toString();
                    QJsonObject state = part["state"].toObject();
                    QString status = state["status"].toString();
                    QJsonObject input = state["input"].toObject();

                    if(status == "running"){
                        // Tool actively executing - show tool use block with input details
                        QString display = toolName;
                        if(input.contains("filePath")){
                            display += ": " + input["filePath"].toString();
                        }else if(input.contains("path")){
                            display += ": " + input["path"].toString();
                        }else if(input.contains("query")){
                            QString q = input["query"].toString();
                            if(q.length() > 60) q = q.left(60) + "...";
                            display += ": " + q;
                        }
                        qDebug() << "[ChatService] tool running:" << toolName;
                        QMetaObject::invokeMethod(this, "streamToolUse", Qt::QueuedConnection,
                                                  Q_ARG(QString, sessionId),
                                                  Q_ARG(QString, display),
                                                  Q_ARG(QString, QString()));
                    }else if(status == "completed"){
                        // Tool finished - show result block
                        QString output = state["output"].toString();
                        QString title = part["title"].toString();
                        QString display = toolName;
                        if(!title.isEmpty()){
                            display += ": " + title;
                        }
                        qDebug() << "[ChatService] tool completed:" << toolName
                                 << "output length=" << output.length();
                        QMetaObject::invokeMethod(this, "streamToolResult", Qt::QueuedConnection,
                                                  Q_ARG(QString, sessionId),
                                                  Q_ARG(QString, display),
                                                  Q_ARG(QString, output));
                    }
                    // pending state is silently ignored (input not yet available)
                }else if(partType == "tool_use" || partType == "tool-use"){
                    QString toolName = part["name"].toString();
                    if(toolName.isEmpty()) toolName = part["toolName"].toString();
                    QJsonObject input = part["input"].toObject();
                    QString inputStr;
                    if(input.isEmpty()){
                        inputStr = part["input"].toString();
                    }else{
                        inputStr = QJsonDocument(input).toJson(QJsonDocument::Indented);
                    }
                    qDebug() << "[ChatService] tool_use:" << toolName;
                    QMetaObject::invokeMethod(this, "streamToolUse", Qt::QueuedConnection,
                                              Q_ARG(QString, sessionId), Q_ARG(QString, toolName), Q_ARG(QString, inputStr));
                }else if(partType == "tool_result" || partType == "tool-result"){
                    QString toolName = part["name"].toString();
                    if(toolName.isEmpty()) toolName = part["toolName"].toString();
                    QString output = part["output"].toString();
                    if(output.isEmpty()) output = part["content"].toString();
                    qDebug() << "[ChatService] tool_result:" << toolName << "output length=" << output.length();
                    QMetaObject::invokeMethod(this, "streamToolResult", Qt::QueuedConnection,
                                              Q_ARG(QString, sessionId), Q_ARG(QString, toolName), Q_ARG(QString, output));
                }
            }else if(type == "session.status"){
                QJsonObject props = payload["properties"].toObject();
                QString sessionId = props["sessionID"].toString();
                QJsonObject status = props["status"].toObject();
                QString statusType = status["type"].toString();
                emit sessionStatusChanged(sessionId, statusType);

                if(statusType == "idle"){
                    qint64 size = m_sessionContentSizes.value(sessionId, 0);
                    if(size > COMPACT_THRESHOLD){
                        qDebug() << "[ChatService] Auto-compacting session:" << sessionId
                                 << "size=" << size << "(threshold=" << COMPACT_THRESHOLD << ")";
                        emit autoCompactionTriggered(sessionId);
                        compactSession(sessionId);
                    }
                    QString error = m_sessionErrors.take(sessionId);
                    emit streamFinished(sessionId, error);
                }
            }else if(type == "session.error"){
                QJsonObject props = payload["properties"].toObject();
                QString sessionId = props["sessionID"].toString();
                QJsonObject errorObj = props["error"].toObject();
                QString errorMsg = errorObj["message"].toString();
                if(errorMsg.isEmpty()) errorMsg = errorObj["name"].toString();
                qDebug() << "[ChatService] session.error for session:" << sessionId << "error:" << errorMsg;
                m_sessionErrors[sessionId] = errorMsg;
            }else if(type == "session.updated"){
                QJsonObject props = payload["properties"].toObject();
                QString sessionId = props["sessionID"].toString();
                QString title = props["info"].toObject()["title"].toString();
                if(!title.isEmpty()){
                    for(int i = 0; i < m_sessions.size(); ++i){
                        if(m_sessions[i].id == sessionId && m_sessions[i].title != title){
                            m_sessions[i].title = title;
                            QMetaObject::invokeMethod(this, "sessionTitleChanged", Qt::QueuedConnection,
                                                      Q_ARG(QString, sessionId), Q_ARG(QString, title));
                            break;
                        }
                    }
                }
            }else if(type == "message.updated"){
                // message updated events are handled via stream deltas
            }else if(type == "session.diff"){
                QJsonObject props = payload["properties"].toObject();
                QString sessionId = props["sessionID"].toString();
                QJsonArray diffArr = props["diff"].toArray();
                if(!diffArr.isEmpty()){
                    QStringList files;
                    for(const auto &d : diffArr){
                        QJsonObject diffObj = d.toObject();
                        QString file = diffObj["file"].toString();
                        if(!file.isEmpty()) files.append(file);
                    }
                    if(!files.isEmpty()){
                        QString summary = QObject::tr("Files modified (%1): %2")
                            .arg(files.size())
                            .arg(files.join(", "));
                        qDebug() << "[ChatService] session.diff:" << sessionId << files;
                        emit sessionDiffChanged(sessionId, summary);
                    }
                }
            }else if(type == "session.next.compaction.started.1"){
                QJsonObject props = payload["properties"].toObject();
                QString sessionId = props["sessionID"].toString();
                QString reason = props["reason"].toString();
                qDebug() << "[ChatService] compaction started for session:" << sessionId << "reason:" << reason;
                emit compactionStarted(sessionId);
            }else if(type == "session.next.compaction.ended"){
                QJsonObject props = payload["properties"].toObject();
                QString sessionId = props["sessionID"].toString();
                m_sessionContentSizes[sessionId] = 0;
                qDebug() << "[ChatService] compaction ended for session:" << sessionId << "(size reset)";
                emit compactionFinished(sessionId);
            }
        }
    }
}

}
