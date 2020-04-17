#pragma once

#include <string>
#include <memory>

#include "transport.h"
#include "epoll_tcp_server.h"


namespace mux {

namespace transport {

class TcpTransport : public Transport {
public:
    TcpTransport()                                     = default;
    TcpTransport(const TcpTransport& other)            = delete;
    TcpTransport& operator=(const TcpTransport& other) = delete;
    TcpTransport(TcpTransport&& other)                 = delete;
    TcpTransport& operator=(TcpTransport&& other)      = delete;

    TcpTransport(const std::string& local_ip, uint16_t local_port, bool is_server);

    ~TcpTransport() override;

public:
    bool Start() override;
    bool Stop() override;
    void RegisterOnRecvCallback(callback_recv_t callback);
    void UnRegisterOnRecvCallback();

    int32_t SendData(const SocketDataPtr& data);
    int32_t SendData(int32_t fd, const std::string& msg);

private:
    bool is_server_ { true }; // role is server as default
    std::string local_ip_;
    uint16_t local_port_ { 0 };

    ETBasePtr sock_{ nullptr };
};

typedef std::shared_ptr<TcpTransport> TcpTransportPtr;

} // end namespace transport

} // end namespace mux
