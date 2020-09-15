#pragma once

#include <string>

#include "epoll/include/tcp_client.h"

namespace mux {

namespace echo {

class EchoTcpClient : public transport::TcpClient {
public:
    virtual ~EchoTcpClient()                             = default;
    EchoTcpClient(const std::string& server_ip, uint16_t server_port);
};

} // end namespace echo

} // end namespace mux
