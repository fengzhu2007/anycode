#include "mcp_handler.h"
#include "storage/project_storage.h"
#include <QDebug>

using namespace ady;

McpHandler::McpHandler(QObject *parent)
    : QObject(parent)
{
}

void McpHandler::registerRoutes()
{
    if (m_routesRegistered) return;
    m_routesRegistered = true;

    Gateway::instance().addPostRoute("/mcp",
        [this](const json &body,
               const std::unordered_map<std::string, std::string> &params)
            -> std::pair<json, int>
        {
            return {handleRequest(body, params), 200};
        });
}

void McpHandler::setWorkspacesProvider(WorkspacesProvider provider)
{
    m_workspacesProvider = std::move(provider);
}

// ========== JSON-RPC Dispatcher ==========

json McpHandler::handleRequest(const json &body,
                               const std::unordered_map<std::string, std::string> & /*params*/)
{
    // Extract JSON-RPC fields
    std::string method = body.value("method", "");
    json id = body.value("id", json(nullptr));
    json params_obj = body.value("params", json::object());

    qDebug() << "[McpHandler] method:" << method.c_str();

    // Notifications (no id) — respond with empty success
    if (method == "notifications/initialized" ||
        method.find("notifications/") == 0) {
        return json::object();
    }

    // Requests (with id)
    if (method == "initialize") {
        return handleInitialize(body);
    }
    if (method == "tools/list") {
        return handleToolsList(body);
    }
    if (method == "tools/call") {
        return handleToolsCall(body);
    }

    return makeError(id, -32601, "Method not found: " + method);
}

// ========== Method Handlers ==========

json McpHandler::handleInitialize(const json &request)
{
    json id = request.value("id", json(nullptr));
    json result;
    result["protocolVersion"] = "2025-03-26";
    result["capabilities"]    = {{"tools", json::object()}};
    result["serverInfo"]      = {
        {"name",    "anycode"},
        {"version", "1.0"}
    };
    return makeResult(id, result);
}

json McpHandler::handleToolsList(const json &request)
{
    json id = request.value("id", json(nullptr));

    json tools = json::array();

    // get_workspaces
    tools.push_back({
        {"name",        "get_workspaces"},
        {"description", "Get the list of open project/workspace directories in the IDE"},
        {"inputSchema", {
            {"type",       "object"},
            {"properties", json::object()},
            {"required",   json::array()}
        }}
    });

    return makeResult(id, {{"tools", tools}});
}

json McpHandler::handleToolsCall(const json &request)
{
    json id = request.value("id", json(nullptr));
    json params_obj = request.value("params", json::object());
    std::string toolName = params_obj.value("name", "");
    json arguments = params_obj.value("arguments", json::object());

    qDebug() << "[McpHandler] tools/call:" << toolName.c_str();

    if (toolName == "get_workspaces") {
        json result = callGetWorkspaces();
        return makeResult(id, result);
    }

    return makeError(id, -32602, "Unknown tool: " + toolName);
}

// ========== Tool Implementations ==========

json McpHandler::callGetWorkspaces()
{
    json workspaces = json::array();

    if (m_workspacesProvider) {
        // Use the IDE-provided callback for real-time workspace list
        QList<WorkspaceInfo> list = m_workspacesProvider();
        for (const auto &ws : list) {
            workspaces.push_back({
                {"name", ws.name.toStdString()},
                {"path", ws.path.toStdString()},
                {"cvs",  ws.cvs.toStdString()}
            });
        }
    } else {
        // Fallback: read from ProjectStorage database
        ProjectStorage storage;
        QList<ProjectRecord> projects = storage.all();
        for (const auto &proj : projects) {
            workspaces.push_back({
                {"name",       proj.name.toStdString()},
                {"path",       proj.path.toStdString()},
                {"cvs",        proj.cvs.toStdString()},
                {"updatetime", proj.updatetime}
            });
        }
    }

    json content;
    content["type"] = "text";
    content["text"] = workspaces.dump(2);

    json result;
    result["content"] = json::array({content});
    return result;
}

// ========== JSON-RPC Helpers ==========

json McpHandler::makeResult(const json &id, const json &result)
{
    return {
        {"jsonrpc", "2.0"},
        {"id",      id},
        {"result",  result}
    };
}

json McpHandler::makeError(const json &id, int code, const std::string &message)
{
    return {
        {"jsonrpc", "2.0"},
        {"id",      id},
        {"error",   {
            {"code",    code},
            {"message", message}
        }}
    };
}
