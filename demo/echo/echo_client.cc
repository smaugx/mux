#include "echo_client.h"

#include <iostream>


namespace mux {

namespace echo {

EchoTcpClient::EchoTcpClient(const std::string& server_ip, uint16_t server_port)
    : MuxSocket(server_ip, server_port) {
    MUX_DEBUG("EchoTcpClient created");
}

} // end namespace echo

} // end namespace mux
