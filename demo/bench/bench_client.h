#pragma once

#include <string>

#include "socket/include/tcp_client.h"

namespace mux {

namespace bench {

class BenchTcpClient : public transport::TcpClient {
public:
    virtual ~BenchTcpClient()                             = default;
    BenchTcpClient(const std::string& server_ip, uint16_t server_port);
};

} // end namespace Bench

} // end namespace mux
