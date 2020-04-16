#pragma once

#include <string>
#include <memory>

#include "transport.h"

namespace mux {

namespace transport {

class TcpTransport : public Transport {
public:
    TcpTransport()                                     = default;
    TcpTransport(const TcpTransport& other)            = delete;
    TcpTransport& operator=(const TcpTransport& other) = delete;
    TcpTransport(TcpTransport&& other)                 = delete;
    TcpTransport& operator=(TcpTransport&& other)      = delete;

    ~TcpTransport() override;

public:
    int Start(const std::string& local_ip, uint16_t local_port) override;

};

typedef std::shared_ptr<TcpTransport> TcpTransportPtr;

} // end namespace transport

} // end namespace mux
