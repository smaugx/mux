#include "bench_socket.h"


namespace mux {

namespace bench {

BenchSocket::BenchSocket(
        int fd,
        const std::string& local_ip,
        uint16_t local_port,
        const std::string& remote_ip,
        uint16_t remote_port)
    : MuxSocket(fd, local_ip, local_port, remote_ip, remote_port) {
}

} // end namespace Bench

} // end namespace mux
