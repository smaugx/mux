#pragma once

#include <string>
#include <map>
#include <mutex>

#include "socket/include/tcp_acceptor.h"

namespace mux {

namespace bench {

class BenchTcpAcceptor : public transport::TcpAcceptor {
public:
    virtual ~BenchTcpAcceptor()                             = default;
    BenchTcpAcceptor(const std::string& local_ip, uint16_t local_port);

public:
    transport::BasicSocket* OnSocketAccept(int32_t cli_fd, const std::string& remote_ip, uint16_t remote_port) override;
    transport::BasicSocket* FindSession(const std::string& ip_port);

private:
    std::mutex session_mutex_;
    std::map<std::string, transport::BasicSocket*> session_;
};

} // end namespace Bench

} // end namespace mux
