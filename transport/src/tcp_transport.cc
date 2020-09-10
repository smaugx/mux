include "transport/include/tcp_transport.h"

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
    : is_server_ { is_server }
      ip_(ip),
      port_(port) {

    assert(!event_trigger_);
    if (is_server_) {
        event_trigger_ = std::make_shared<EventTriggerSvr>(ip, port);
    } else {
        event_trigger_ = std::make_shared<EventTriggerCli>(ip, port);
    }
    assert(event_trigger_);
}

TcpTransport::~TcpTransport() {
    Stop();
}


bool TcpTransport::Start() {
    if (ip_.empty() || port_ == 0) {
        MUX_ERROR("invalid ip:{0} or port:{1}", ip_, port_);
        return false;
    }
    if (!event_trigger_) {
        MUX_ERROR("event_trigger_ invalid");
        return false;
    }

    return event_trigger_->Start();
}

bool TcpTransport::Stop() {
    if (event_trigger_) {
        event_trigger_->Stop();
        event_trigger_ = nullptr;
    }
    MUX_INFO("TcpTransport Stop");
    return true;
}

int32_t TcpTransport::SendData(const PacketPtr& packet) {
    return event_trigger_->GetSocket()->SendData(packet);
}

int32_t TcpTransport::SendData(const std::string& msg) {
    return event_trigger_->GetSocket()->SendData(msg);
}
}

void TcpTransport::RegisterOnRecvCallback(callback_recv_t callback) {
    assert(callback);
    event_trigger_->RegisterOnRecvCallback(callback);
}

void TcpTransport::UnRegisterOnRecvCallback() {
    event_trigger_->UnRegisterOnRecvCallback();
}

void TcpTransport::RegisterOnAcceptCallback(callback_accept_t callback) {
    assert(callback);
    event_trigger_->RegisterOnAcceptCallback(callback);
}


} // end namespace transport

} // end namespace mux
