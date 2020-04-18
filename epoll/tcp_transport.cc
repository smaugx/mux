#include "tcp_transport.h"

#include <cassert>

#include <iostream>

#include "epoll_tcp_server.h"
#include "epoll_tcp_client.h"



namespace mux {

namespace transport {

TcpTransport::TcpTransport(
        const std::string& ip,
        uint16_t port,
        bool is_server)
    : is_server_ { is_server },
      ip_ { ip },
      port_ { port } {

    assert(!sock_);
    if (is_server_) {
        sock_ = std::make_shared<ETServer>(ip_, port_);
    } else {
        sock_ = std::make_shared<ETClient>(ip_, port_);
    }
    assert(sock_);
}

TcpTransport::~TcpTransport() {
}


bool TcpTransport::Start() {
    if (ip_.empty() || port_ == 0) {
        std::cout << "invalid ip: " << ip_ << " port: " << port_ << std::endl;
        return false;
    }
    if (!sock_) {
        return false;
    }

    return sock_->Start();
}

bool TcpTransport::Stop() {
    if (sock_) {
        sock_->Stop();
        sock_ = nullptr;
    }
    return true;
}

int32_t TcpTransport::SendData(const SocketDataPtr& data) {
    return sock_->SendData(data);
}

int32_t TcpTransport::SendData(int32_t fd, const std::string& msg) {
    auto data = std::make_shared<SocketData>(fd, msg);
    return sock_->SendData(data);
}

void TcpTransport::RegisterOnRecvCallback(callback_recv_t callback) {
    assert(callback);
    sock_->RegisterOnRecvCallback(callback);
}

void TcpTransport::UnRegisterOnRecvCallback() {
    sock_->UnRegisterOnRecvCallback();
}


} // end namespace transport

} // end namespace mux
