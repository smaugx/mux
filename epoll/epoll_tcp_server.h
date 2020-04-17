#pragma once

#include <string>
#include <memory>
#include <thread>

namespace mux {

namespace transport {

typedef struct SocketData {
public:
    SocketData()
        : fd_ { -1 },
          msg_ { "" } {}
    SocketData(int32_t fd, const std::string& msg)
        : fd_ { fd },
          msg_ { msg } {}

    int32_t fd_ { -1 };
    std::string msg_;
} SocketData;

typedef std::shared_ptr<SocketData> SocketDataPtr;

using callback_recv_t = std::function<void(const SocketDataPtr& data)>;

class EpollTcpServer {
public:
    EpollTcpServer()                                       = default;
    EpollTcpServer(const EpollTcpServer& other)            = delete;
    EpollTcpServer& operator=(const EpollTcpServer& other) = delete;
    EpollTcpServer(EpollTcpServer&& other)                 = delete;
    EpollTcpServer& operator=(EpollTcpServer&& other)      = delete;
    virtual ~EpollTcpServer();

    EpollTcpServer(const std::string& local_ip, uint16_t local_port);

public:
    bool Start();
    bool Stop();
    int32_t SendData(const SocketDataPtr& data);
    void RegisterOnRecvCallback(callback_recv_t callback);
    void UnRegisterOnRecvCallback();

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
