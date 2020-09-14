#include "echo_tcp_acceptor.h"
#include "echo_socket.h"

#include <iostream>

#include "mbase/include/mux_log.h"


namespace mux {

namespace echo {

EchoTcpAcceptor::EchoTcpAcceptor(const std::string& local_ip, uint16_t local_port)
    : local_ip_(local_ip),
      local_port_(local_port) {}


SocketBase* EchoTcpAcceptor::OnSocketAccept(int32_t cli_fd, std::string remote_ip, uint16_t remote_port) {
    MUX_DEBUG("in EchoTcpAcceptor OnSocketAccept");
    SocketBase* new_sock = new EchoSocket(cli_fd, local_ip_, local_port_, remote_ip, remote_port);
    if (!new_sock) {
        MUX_ERROR("error create muxsocket");
        return nullptr;
    }

    return TcpAcceptor::RecordNewConnection(new_sock);
}

} // end namespace echo

} // end namespace mux
