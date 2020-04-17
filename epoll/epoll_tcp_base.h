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

class EpollTcpBase {
public:
    EpollTcpBase()                                     = default;
    EpollTcpBase(const EpollTcpBase& other)            = delete;
    EpollTcpBase& operator=(const EpollTcpBase& other) = delete;
    EpollTcpBase(EpollTcpBase&& other)                 = delete;
    EpollTcpBase& operator=(EpollTcpBase&& other)      = delete;
    virtual ~EpollTcpBase()  = 0 ;

public:
    bool Start() = 0;
    bool Stop()  = 0;
    int32_t SendData(const SocketDataPtr& data) = 0;
    void RegisterOnRecvCallback(callback_recv_t callback) = 0;
    void UnRegisterOnRecvCallback() = 0;
};

using ETBase = EpollTcpBase;

typedef std::shared_ptr<ETBase> ETBasePtr;

}

}
