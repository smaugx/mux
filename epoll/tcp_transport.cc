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


bool TcpTransport::Start() {
    if (local_ip_.empty() || local_port_ == 0) {
        std::cout << "invalid local_ip: " << local_ip_ << " local_port: " << local_port_ << std::endl;
        return false;
    }

    assert(!sock_);
    if (is_server_) {
        sock_ = std::make_shared<ETServer>(local_ip_, local_port_);
    } else {
        //sock_ = std::make_shared<ETClient>(local_ip_, local_port_);
    }

    if (!sock_) {
        std::cout << "sock create failed!" << std::endl;
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




}

}
