#include "echo_client.h"

#include <iostream>

#include "mbase/include/mux_log.h"


namespace mux {

namespace echo {

EchoTcpClient::EchoTcpClient(const std::string& server_ip, uint16_t server_port)
    : transport::TcpClient(server_ip, server_port) {
    MUX_DEBUG("EchoTcpClient created");
}

} // end namespace echo

} // end namespace mux
