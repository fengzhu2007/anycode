#include "gateway_http_server.h"
#include "mcp/mcp_handler.h"
#include "panes/resource_manager/resource_manager_model.h"
#include "panes/resource_manager/resource_manager_model_item.h"
#include "modules/options/ai_settings.h"
#include "modules/options/options_settings.h"
#include "modules/options/network_settings.h"
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
    auto config = GatewayConfig();
#ifndef Q_DEBUG
    config.port = 3459;
#endif
    // Apply proxy configuration from NetworkSettings if Gateway is enabled
    {
        auto networkSetting = ady::OptionsSettings::getInstance()->networkSettings();
        if (networkSetting.m_gatewayEnabled && !networkSetting.m_host.isEmpty()) {
            config.proxy.host = networkSetting.m_host.toStdString();
            config.proxy.port = static_cast<uint16_t>(networkSetting.m_port);
            config.proxy.username = networkSetting.m_username.toStdString();
            config.proxy.password = networkSetting.m_password.toStdString();
            qDebug() << "[GatewayHttpServer] Proxy configured:"
                     << networkSetting.m_host << ":" << networkSetting.m_port;
        }
    }
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
            // Explicit headers (matching: curl -H "Content-Type: application/json" -H "Authorization: Bearer KEY")
            out.headers["Content-Type"]  = "application/json";
            out.headers["Authorization"] = "Bearer " + out.apiKey;
            // Map gateway model name to actual Gemini model
            out.body   = requestBody;
            out.body["model"] = "gemini-3.5-flash";
            // Remove non-standard fields that conflict with Gemini's OpenAI-compatible endpoint
            out.body.erase("maxTokens");
            out.body.erase("topP");
            out.body.erase("topK");
            out.body.erase("frequencyPenalty");
            out.body.erase("presencePenalty");
            // Gemini requires thought_signature in function calls when thinking is enabled.
            // Add placeholder signature to any tool_calls missing it.
            if (out.body.contains("messages") && out.body["messages"].is_array()) {
                for (auto &msg : out.body["messages"]) {
                    if (msg.contains("tool_calls") && msg["tool_calls"].is_array()) {
                        for (auto &tc : msg["tool_calls"]) {
                            if (tc.contains("function") && !tc.contains("thought_signature")) {
                                tc["thought_signature"] = "";
                            }
                        }
                    }
                }
            }
            return true;
        });

    // nvidia-deepseek-v4-pro-0813 → NVIDIA API (DeepSeek V4 Pro)
    mutableConfig.models.push_back({"nvidia-deepseek-v4-pro-0813", "nvidia"});
    LlmProxy::registerResolver("nvidia-deepseek-v4-pro-0813",
        [](const std::string &/*model*/, const json &requestBody, RouteConfig &out) -> bool {
            auto setting = ady::OptionsSettings::getInstance()->aiSettings();
            QString key = setting.m_nvidiaApiKey;
            if (key.isEmpty()) {
                qWarning() << "[GatewayHttpServer] NVIDIA API KEY not set in AI settings";
                return false;
            }
            out.url    = "https://integrate.api.nvidia.com/v1/chat/completions";
            out.apiKey = key.toStdString();
            out.headers["Content-Type"]  = "application/json";
            out.headers["Authorization"] = "Bearer " + out.apiKey;
            out.body   = requestBody;
            out.body["model"] = "deepseek-ai/deepseek-v4-pro-0813";
            // Disable thinking by default (matching curl reference)
            if (!out.body.contains("chat_template_kwargs")) {
                out.body["chat_template_kwargs"] = {{"thinking", false}};
            }
            // Remove non-standard camelCase fields
            out.body.erase("maxTokens");
            out.body.erase("topP");
            out.body.erase("topK");
            out.body.erase("frequencyPenalty");
            out.body.erase("presencePenalty");
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
    // If gateway is still starting, wait for it to finish
    if (m_startWatcher && m_startWatcher->isRunning()) {
        LlmProxy::shutdown();
        m_startWatcher->waitForFinished();
    }
    if (!m_running && !Gateway::instance().isRunning()) {
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
