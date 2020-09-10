#pragma once

#include <string>
#include <memory>

#include "transport/include/transport.h"
#include "epoll/include/event_trigger.h"


namespace mux {

namespace transport {

class TcpTransport : public Transport {
public:
    TcpTransport()                                     = default;
    TcpTransport(const TcpTransport& other)            = delete;
    TcpTransport& operator=(const TcpTransport& other) = delete;
    TcpTransport(TcpTransport&& other)                 = delete;
    TcpTransport& operator=(TcpTransport&& other)      = delete;

    TcpTransport(const std::string& ip, uint16_t port, bool is_server);

    ~TcpTransport() override;

public:
    bool Start() override;
    bool Stop() override;
    void RegisterOnRecvCallback(callback_recv_t callback);
    void RegisterOnAcceptCallback(callback_accept_t callback);
    void UnRegisterOnRecvCallback();

    int32_t SendData(const PacketPtr& packet);
    int32_t SendData(const std::string& msg);

private:
    bool is_server_ { true }; // role is server as default
    std::string ip_; // for server set local bind ip:port; for client set server ip:port
    uint16_t port_ { 0 };
    EventTriggerBasePtr event_trigger_ { nullptr };
};

typedef std::shared_ptr<TcpTransport> TcpTransportPtr;

} // end namespace transport

} // end namespace mux
