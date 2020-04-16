#pragma once

namespace mux {

namespace transport {

class EpollTcpSocket {
public:
    EpollTcpSocket()                                       = default;
    EpollTcpSocket(const EpollTcpSocket& other)            = delete;
    EpollTcpSocket& operator=(const EpollTcpSocket& other) = delete;
    EpollTcpSocket(EpollTcpSocket&& other)                 = delete;
    EpollTcpSocket& operator=(EpollTcpSocket&& other)      = delete;
    virtual ~EpollTcpSocket();

    EpollTcpSocket(const std::string& local_ip, uint16_t local_port);

public:
    bool Init();
    void Start();
    bool Stop();

protected:
    int32_t CreateAndBind();
    int32_t MakeSocketNonBlock(int32_t fd);
    int32_t Listen(int32_t listenfd);
    int32_t UpdateEpollEvents(int efd, int fd, int events, int op);
    void StartLoop();


private:
    std::string local_ip_;
    uint16_t local_port_ { 0 };
    int32_t handle_ { -1 }; // listenfd
    int32_t efd_ { -1 }; // epoll fd
    std::shared_ptr<std::thread> th_loop_ { nullptr };
    bool loop_flag_ { true };
};

using ETsocket = EpollTcpSocket;

typedef std::shared_ptr<ETsocket> ETsocketPtr;

}

}
