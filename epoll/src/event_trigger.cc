#include "epoll/include/event_trigger.h"


namespace mux {

namespace transport {

EventTriggerCli::EventTriggerCli(const std::string& server_ip, uint16_t server_port) {
    reactor_ = std::make_shared<EpollTcpClient>(server_ip, server_port);
    assert(reactor_);
}

EventTriggerCli::~EventTriggerCli() {
    reactor_->Stop();
    reactor_ = nullptr;
}

bool EventTriggerCli::Start() {
    reactor_->Start();
}

bool EventTriggerCli::Stop() {
    reactor_->Stop();
}

void EventTriggerCli::RegisterOnRecvCallback(callback_recv_t callback) {
    reactor_->RegisterOnRecvCallback(callback);
}

void EventTriggerCli::UnRegisterOnRecvCallback() {
    reactor_->UnRegisterOnRecvCallback();
}

void EventTriggerCli::RegisterOnAcceptCallback(callback_accept_t callback) {
    reactor_->RegisterOnAcceptCallback(callback);
}

SocketPtr EventTriggerCli::GetSocket() {
    reactor_->GetSocket();
}


EventTriggerSvr::EventTriggerSvr(const std::string& local_ip, uint16_t local_port) {
    reactor_ = std::make_shared<EpollTcpServer>(local_ip, local_port);
    assert(reactor_);
}

EventTriggerSvr::~EventTriggerSvr() {
    reactor_->Stop();
    reactor_ = nullptr;
}

bool EventTriggerSvr::Start() {
    reactor_->Start();
}

bool EventTriggerSvr::Stop() {
    reactor_->Stop();
}

void EventTriggerSvr::RegisterOnRecvCallback(callback_recv_t callback) {
    reactor_->RegisterOnRecvCallback(callback);
}

void EventTriggerSvr::UnRegisterOnRecvCallback() {
    reactor_->UnRegisterOnRecvCallback();
}

void EventTriggerSvr::RegisterOnAcceptCallback(callback_accept_t callback) {
    reactor_->RegisterOnAcceptCallback(callback);
}

SocketPtr EventTriggerSvr::GetSocket() {
    reactor_->GetSocket();
}


} // end namespace transport

} // end namespace mux
