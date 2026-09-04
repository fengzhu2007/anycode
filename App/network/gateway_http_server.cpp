#include "gateway_http_server.h"
#include "mcp/mcp_handler.h"
#include "panes/resource_manager/resource_manager_model.h"
#include "panes/resource_manager/resource_manager_model_item.h"
#include "modules/options/ai_settings.h"
#include "modules/options/options_settings.h"
#include <llmproxy.h>
#include <QDebug>
#include <QtConcurrent>

GatewayHttpServer::GatewayHttpServer(QObject *parent)
    : QObject(parent)
{
}

GatewayHttpServer::~GatewayHttpServer()
{
    stop();
}

GatewayHttpServer &GatewayHttpServer::instance()
{
    static GatewayHttpServer inst;
    return inst;
}

void GatewayHttpServer::start()
{
    //
    auto config = GatewayConfig();
#ifndef Q_DEBUG
    config.port = 3459;
#endif
    start(config);
}

void GatewayHttpServer::start(const GatewayConfig &config)
{
    if (m_running) {
        qWarning() << "[GatewayHttpServer] Already running, port=" << port();
        return;
    }

    if (m_startWatcher && m_startWatcher->isRunning()) {
        qWarning() << "[GatewayHttpServer] Start already in progress.";
        return;
    }

    // Register MCP routes before Gateway starts (must be done in main thread)
    if (!m_mcpHandler) {
        m_mcpHandler = new McpHandler(this);
        // Wire up workspace provider: read from ResourceManagerModel (current open projects)
        m_mcpHandler->setWorkspacesProvider([]() -> QList<McpHandler::WorkspaceInfo> {
            QList<McpHandler::WorkspaceInfo> result;
            auto *model = ady::ResourceManagerModel::getInstance();
            if (!model) return result;
            auto *root = model->rootItem();
            if (!root) return result;
            for (int i = 0; i < root->childrenCount(); ++i) {
                auto *item = root->childAt(i);
                if (!item) continue;
                McpHandler::WorkspaceInfo ws;
                ws.name = item->title();
                ws.path = item->path();
                if (!ws.path.isEmpty())
                    result.append(ws);
            }
            return result;
        });
    }
    m_mcpHandler->registerRoutes();

    // Register LLM proxy resolvers for custom models
    auto mutableConfig = config;

    // Reset proxy shutdown flag (in case of restart after previous stop)
    LlmProxy::resetShutdown();

    // gateway-gemini-2.5 → Google Gemini (OpenAI-compatible endpoint)
    mutableConfig.models.push_back({"gateway-gemini-3.5-flash", "google"});
    LlmProxy::registerResolver("gateway-gemini-3.5-flash",
        [](const std::string &/*model*/, const json &requestBody, RouteConfig &out) -> bool {
            // Read API key from AISettings
            auto setting = ady::OptionsSettings::getInstance()->aiSettings();
            QString key = setting.m_geminiApiKey;
            if (key.isEmpty()) {
                qWarning() << "[GatewayHttpServer] Gemini API KEY not set in AI settings";
                return false;
            }
            out.url    = "https://generativelanguage.googleapis.com/v1beta/openai/chat/completions";
            out.apiKey = key.toStdString();
            // Map gateway model name to actual Gemini model
            out.body   = requestBody;
            out.body["model"] = "gemini-3.5-flash";
            return true;
        });

    qDebug() << "[GatewayHttpServer] Starting gateway async..."
             << "host=" << mutableConfig.host.c_str() << "port=" << mutableConfig.port;

    auto future = QtConcurrent::run([mutableConfig]() -> bool {
        return Gateway::instance().start(mutableConfig);
    });

    if (!m_startWatcher) {
        m_startWatcher = new QFutureWatcher<bool>(this);
        connect(m_startWatcher, &QFutureWatcher<bool>::finished, this, [this]() {
            bool ok = m_startWatcher->result();
            if (ok) {
                m_running = true;
                uint16_t p = Gateway::instance().port();
                qDebug() << "[GatewayHttpServer] Gateway started on port" << p;
                emit started(p);
            } else {
                qWarning() << "[GatewayHttpServer] Failed to start gateway.";
                emit startFailed(QStringLiteral("Gateway::start() returned false"));
            }
        });
    }

    m_startWatcher->setFuture(future);
}

void GatewayHttpServer::stop()
{
    if (!m_running) {
        return;
    }
    qDebug() << "[GatewayHttpServer] Stopping gateway...";
    Gateway::instance().stop();
    m_running = false;
    // Reset MCP flag so routes can be re-registered on next start
    if (m_mcpHandler) {
        delete m_mcpHandler;
        m_mcpHandler = nullptr;
    }
    qDebug() << "[GatewayHttpServer] Gateway stopped.";
    emit stopped();
}

bool GatewayHttpServer::isRunning() const
{
    return m_running;
}

uint16_t GatewayHttpServer::port() const
{
    return Gateway::instance().port();
}
