#include "epoll/include/event_trigger.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cassert>

#include <iostream>

#include "mbase/include/mux_log.h"

namespace mux {

namespace transport {

EventTrigger::EventTrigger() {}

EventTrigger::EventTrigger(int ep_num)
    : ep_num_(ep_num) {}

EventTrigger::~EventTrigger() {
    Stop();
}

bool EventTrigger::Start() {
    for (int i = 0; i < ep_num_; ++i) {
        auto epoll_reactor = std::make_shared<EpollReactor>(i);
        assert(epoll_reactor);
        epoll_reactor->Start();
        reactor_vec_.push_back(epoll_reactor);
    }
    // and then register fd to epoll tree
    MUX_INFO("start {0} reactor", ep_num_);
    return true;
}

bool EventTrigger::Stop() {
    int i = 0;
    for (const auto& reactor : reactor_vec_) {
        reactor->Stop();
        MUX_DEBUG("stop reactor:{0}", i);
        ++i;
    }
    reactor_vec_.clear();
    return true;
}


void EventTrigger::RegisterOnAcceptCallback(callback_accept_t callback) {
    int i = 0;
    for (const auto& reactor : reactor_vec_) {
        reactor->RegisterOnAcceptCallback(callback);
        MUX_DEBUG("register accept callback reactor:{0}", i);
        ++i;
    }
}


int32_t EventTrigger::RegisterDescriptor(void* ptr, int events) {
    int i = 0;
    for (const auto& reactor : reactor_vec_) {
        reactor->RegisterDescriptor(ptr,  events);
        MUX_DEBUG("register descriptor to reactor:{0}", i);
        ++i;
    }
    return 0;
}


} // end namespace transport

} // end namespace mux
