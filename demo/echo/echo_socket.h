#pragma once

#include "socket/include/socket_imp.h"

namespace mux {

namespace echo {

class EchoSocket : public transport::MuxSocket {
public:
    EchoSocket(
            int fd,
            const std::string& local_ip,
            uint16_t local_port,
            const std::string& remote_ip,
            uint16_t remote_port);
    virtual ~EchoSocket() = default;
};

} // end namespace echo

} // end namespace mux
