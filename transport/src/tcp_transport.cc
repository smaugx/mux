#include "transport/include/tcp_transport.h"

#include <cassert>

#include <iostream>

#include "epoll/include/epoll_tcp_server.h"
#include "epoll/include/epoll_tcp_client.h"
#include "mbase/mux_log.h"



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
    Stop();
}


bool TcpTransport::Start() {
    if (ip_.empty() || port_ == 0) {
        MUX_ERROR("invalid ip:{0} or port:{1}", ip_, port_);
        return false;
    }
    if (!sock_) {
        MUX_ERROR("sock_ invalid");
        return false;
    }

    return sock_->Start();
}

bool TcpTransport::Stop() {
    if (sock_) {
        sock_->Stop();
        sock_ = nullptr;
    }
    MUX_INFO("TcpTransport Stop");
    return true;
}

int32_t TcpTransport::SendData(const PacketPtr& packet) {
    return sock_->SendData(packet);
}

int32_t TcpTransport::SendData(int32_t fd, const std::string& msg) {
    auto packet = std::make_shared<Packet>(fd, msg);
    return sock_->SendData(packet);
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
