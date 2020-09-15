#include "echo_tcp_acceptor.h"
#include "echo_socket.h"

#include <iostream>

#include "mbase/include/mux_log.h"


namespace mux {

namespace echo {

EchoTcpAcceptor::EchoTcpAcceptor(const std::string& local_ip, uint16_t local_port)
    : TcpAcceptor(local_ip, local_port) {}



transport::BasicSocket* EchoTcpAcceptor::OnSocketAccept(int32_t cli_fd, const std::string& remote_ip, uint16_t remote_port) {
    MUX_DEBUG("in EchoTcpAcceptor OnSocketAccept");
    transport::BasicSocket* new_sock = new EchoSocket(cli_fd, local_ip_, local_port_, remote_ip, remote_port);
    if (!new_sock) {
        MUX_ERROR("error create muxsocket");
        return nullptr;
    }

    {
        std::unique_lock<std::mutex> lock(session_mutex_);
        std::string key = remote_ip + ":" + std::to_string(remote_port);
        session_[key] = new_sock;
        MUX_DEBUG("add new connection {0}, now size:{1}", key, session_.size());
    }

    return transport::TcpAcceptor::ManageNewConnection(new_sock);
}


transport::BasicSocket* EchoTcpAcceptor::FindSession(const std::string& ip_port) {
    {
        std::unique_lock<std::mutex> lock(session_mutex_);
        auto ifind = session_.find(ip_port);
        if (ifind == session_.end()) {
            return nullptr;
        } else {
            return ifind->second;
        }
    }
}

} // end namespace echo

} // end namespace mux
