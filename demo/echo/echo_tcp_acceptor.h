#pragma once

#include <string>
#include <map>
#include <mutex>

#include "epoll/include/tcp_acceptor.h"

namespace mux {

namespace echo {

class EchoTcpAcceptor : public TcpAcceptor {
public:
    virtual ~EchoTcpAcceptor()                             = default;
    EchoTcpAcceptor(const std::string& local_ip, uint16_t local_port);

public:
    SocketBase* OnSocketAccept(int32_t cli_fd, const std::string& remote_ip, uint16_t remote_port) override;

private:
};

} // end namespace echo

} // end namespace mux
