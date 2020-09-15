#pragma once

#include "socket/include/socket_imp.h"

namespace mux {

namespace bench {

class BenchSocket : public transport::MuxSocket {
public:
    BenchSocket(
            int fd,
            const std::string& local_ip,
            uint16_t local_port,
            const std::string& remote_ip,
            uint16_t remote_port);
    virtual ~BenchSocket() = default;
};

} // end namespace Bench

} // end namespace mux
