#pragma once

#include <thread>

#include "epoll/include/socket_imp.h"
#include "mbase/packet.h"

namespace mux {

namespace transport {


class EpollTcpBase {
public:
    EpollTcpBase()                                     = default;
    EpollTcpBase(const EpollTcpBase& other)            = delete;
    EpollTcpBase& operator=(const EpollTcpBase& other) = delete;
    EpollTcpBase(EpollTcpBase&& other)                 = delete;
    EpollTcpBase& operator=(EpollTcpBase&& other)      = delete;
    virtual ~EpollTcpBase()                            = default; 

public:
    virtual bool Start() = 0;
    virtual bool Stop()  = 0;
    virtual void RegisterOnRecvCallback(callback_recv_t callback) = 0;
    virtual void UnRegisterOnRecvCallback() = 0;
    virtual void RegisterOnAcceptCallback(callback_accept_t callback) = 0;
};

using ETBase = EpollTcpBase;

typedef std::shared_ptr<ETBase> ETBasePtr;

} // end namespace transport

} // end namespace mux
