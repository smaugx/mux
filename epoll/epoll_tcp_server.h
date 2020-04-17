#pragma once

#include <string>
#include <memory>
#include <thread>

#include "epoll_tcp_base.h"

namespace mux {

namespace transport {

class EpollTcpServer : public ETBase {
public:
    EpollTcpServer()                                       = default;
    EpollTcpServer(const EpollTcpServer& other)            = delete;
    EpollTcpServer& operator=(const EpollTcpServer& other) = delete;
    EpollTcpServer(EpollTcpServer&& other)                 = delete;
    EpollTcpServer& operator=(EpollTcpServer&& other)      = delete;
    ~EpollTcpServer() override;

    EpollTcpServer(const std::string& local_ip, uint16_t local_port);

public:
    bool Start() override;
    bool Stop() override;
    int32_t SendData(const SocketDataPtr& data) override;
    void RegisterOnRecvCallback(callback_recv_t callback) override;
    void UnRegisterOnRecvCallback() override;

protected:
    int32_t CreateEpoll();
    int32_t CreateAndBind();
    int32_t MakeSocketNonBlock(int32_t fd);
    int32_t Listen(int32_t listenfd);
    int32_t UpdateEpollEvents(int efd, int op, int fd, int events);
    void OnSocketAccept();
    void OnSocketRead(int32_t fd);
    void OnSocketWrite(int32_t fd);
    void EpollLoop();


private:
    std::string local_ip_;
    uint16_t local_port_ { 0 };
    int32_t handle_ { -1 }; // listenfd
    int32_t efd_ { -1 }; // epoll fd
    std::shared_ptr<std::thread> th_loop_ { nullptr };
    bool loop_flag_ { true };
    callback_recv_t recv_callback_ { nullptr };
};

using ETServer = EpollTcpServer;

typedef std::shared_ptr<ETServer> ETServerPtr;

}

}
