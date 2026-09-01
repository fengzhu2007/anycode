#pragma once

#include <QObject>
#include <QList>
#include <QJsonObject>
#include <functional>
#include <gateway.h>

/**
 * MCP (Model Context Protocol) handler
 *
 * Implements a minimal MCP server as a custom route on the Gateway.
 * Supports JSON-RPC 2.0 over HTTP POST at /mcp.
 *
 * Currently supported methods:
 *   - initialize       : MCP handshake
 *   - notifications/initialized
 *   - tools/list       : enumerate available tools
 *   - tools/call       : invoke a tool
 *
 * Tools:
 *   - get_workspaces   : return the list of open project directories
 */
class McpHandler : public QObject
{
    Q_OBJECT
public:
    explicit McpHandler(QObject *parent = nullptr);

    /// Register all MCP routes on the Gateway (call before Gateway::start)
    void registerRoutes();

    /// Workspace info returned by the provider callback
    struct WorkspaceInfo {
        QString name;
        QString path;
        QString cvs;
    };

    /// Callback type: returns the current workspace list from the IDE
    using WorkspacesProvider = std::function<QList<WorkspaceInfo>()>;

    /// Set the callback that provides the current workspace list.
    /// When set, get_workspaces tool uses this instead of ProjectStorage.
    void setWorkspacesProvider(WorkspacesProvider provider);

private:
    bool m_routesRegistered = false;
    WorkspacesProvider m_workspacesProvider;

    /// Main JSON-RPC dispatcher
    json handleRequest(const json &body,
                       const std::unordered_map<std::string, std::string> &params);

    // JSON-RPC method handlers
    json handleInitialize(const json &request);
    json handleToolsList(const json &request);
    json handleToolsCall(const json &request);

    // Tool implementations
    json callGetWorkspaces();

    // JSON-RPC helpers
    static json makeResult(const json &id, const json &result);
    static json makeError(const json &id, int code, const std::string &message);
};
