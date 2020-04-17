#include "tcp_transport.h"

#include "epoll_tcp_server.h"



namespace mux {

namespace transport {

TcpTransport::TcpTransport(
        const std::string& local_ip,
        uint16_t local_port,
        bool is_server)
    : is_server_ { is_server },
      local_ip_ { local_ip },
      local_port_ { local_port } {
}

TcpTransport::~TcpTransport() {
}

int TcpTransport::Start() {
}

}

}
