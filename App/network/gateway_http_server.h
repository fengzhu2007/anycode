#pragma once

#include <QObject>
#include <QString>
#include <QFutureWatcher>
#include <cstdint>
#include <gateway.h>

class McpHandler;

/**
 * Gateway HTTP Server 单例
 *
 * 负责初始化和管理 libgatewap 网关服务。
 * 启动/停止均为异步操作，不阻塞 UI 线程。
 * 与 App/network 下的 FTP/SFTP 等网络模块无关。
 */
class GatewayHttpServer : public QObject
{
    Q_OBJECT
public:
    static GatewayHttpServer &instance();

    /// 异步启动网关服务（使用默认配置），立即返回
    void start();

    /// 异步启动网关服务（自定义配置），立即返回
    void start(const GatewayConfig &config);

    /// 异步停止网关服务
    void stop();

    /// 是否正在运行
    bool isRunning() const;

    /// 获取当前服务端口
    uint16_t port() const;

signals:
    /// 网关启动成功
    void started(uint16_t port);

    /// 网关启动失败
    void startFailed(const QString &reason);

    /// 网关已停止
    void stopped();

private:
    explicit GatewayHttpServer(QObject *parent = nullptr);
    ~GatewayHttpServer() override;
    GatewayHttpServer(const GatewayHttpServer &) = delete;
    GatewayHttpServer &operator=(const GatewayHttpServer &) = delete;

    bool m_running = false;
    QFutureWatcher<bool> *m_startWatcher = nullptr;
    McpHandler *m_mcpHandler = nullptr;
};
