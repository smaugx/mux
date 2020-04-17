#pragma once

#include <string>
#include <memory>

#include "transport.h"


namespace mux {

namespace transport {

class ETServer;
typedef std::shared_ptr<ETServer> ETServerPtr;

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
    int Start() override;

private:
    bool is_server_ { true }; // role is server as default
    std::string local_ip_;
    uint16_t local_port_ { 0 };

    ETServerPtr server_ { nullptr };
};

typedef std::shared_ptr<TcpTransport> TcpTransportPtr;

} // end namespace transport

} // end namespace mux
