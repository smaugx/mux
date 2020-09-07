#pragma once

#include <string>
#include <memory>
#include <thread>

#include "epoll/include/epoll_tcp_base.h"

namespace mux {

namespace transport {

class EpollTcpClient : public ETBase {
public:
    EpollTcpClient()                                       = default;
    EpollTcpClient(const EpollTcpClient& other)            = delete;
    EpollTcpClient& operator=(const EpollTcpClient& other) = delete;
    EpollTcpClient(EpollTcpClient&& other)                 = delete;
    EpollTcpClient& operator=(EpollTcpClient&& other)      = delete;
    ~EpollTcpClient() override;

    EpollTcpClient(const std::string& server_ip, uint16_t server_port);

public:
    bool Start() override;
    bool Stop() override;
    int32_t SendData(const PacketPtr& packet) override;
    void RegisterOnRecvCallback(callback_recv_t callback) override;
    void UnRegisterOnRecvCallback() override;

protected:
    int32_t CreateEpoll();
    int32_t CreateSocket();
    int32_t Connect(int32_t listenfd);
    int32_t MakeSocketNonBlock(int32_t fd);
    int32_t UpdateEpollEvents(int efd, int op, int fd, int events);
    void OnSocketRead(int32_t fd);
    void OnSocketWrite(int32_t fd);
    void EpollLoop();


private:
    std::string server_ip_;
    uint16_t server_port_ { 0 };
    int32_t handle_ { -1 }; // client fd
    int32_t efd_ { -1 }; // epoll fd
    std::shared_ptr<std::thread> th_loop_ { nullptr };
    bool loop_flag_ { true };
    callback_recv_t recv_callback_ { nullptr };
};

using ETClient = EpollTcpClient;

typedef std::shared_ptr<ETClient> ETClientPtr;

}

}
