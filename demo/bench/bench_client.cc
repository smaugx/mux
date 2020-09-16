#include "Bench_client.h"

#include <iostream>

#include "mbase/include/mux_log.h"


namespace mux {

namespace bench {

BenchTcpClient::BenchTcpClient(const std::string& server_ip, uint16_t server_port)
    : transport::TcpClient(server_ip, server_port) {
    MUX_DEBUG("BenchTcpClient created");
}

} // end namespace Bench

} // end namespace mux
