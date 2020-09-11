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

#include "mbase/mux_log.h"

namespace mux {

namespace transport {

EventTrigger::EventTrigger() {}

EventTrigger::EventTrigger(int32_t handle, int ep_num)
    : handle_(handle),
      ep_num_(ep_num) {}

EventTrigger::~EventTrigger() {
    Stop();
}

bool EventTrigger::Start() {
    if (handle_ != -1) {
        for (int i = 0; i < ep_num_; ++i) {
            auto epoll_reactor = std::make_shared<EpollReactor>(i, handle_);
            assert(epoll_reactor);
            epoll_reactor->Start();
            reactor_vec_.push_back(epoll_reactor);
        }
    } else {
        auto epoll_reactor = std::make_shared<EpollReactor>();
        assert(epoll_reactor);
        epoll_reactor->Start();
        reactor_vec_.push_back(epoll_reactor);
        // TODO(smaug) add to epoll tree
    }
}

bool EventTrigger::Stop() {
    int i = 0;
    for (const auto& reactor : reactor_vec_) {
        reactor->Stop();
        MUX_DEBUG("stop reactor:{0}", i);
        ++i;
    }
}


void EventTrigger::RegisterOnAcceptCallback(callback_accept_t callback) {
    int i = 0;
    for (const auto& reactor : reactor_vec_) {
        reactor->RegisterOnAcceptCallback(callback);
        MUX_DEBUG("register accept callback reactor:{0}", i);
        ++i;
    }
}


} // end namespace transport

} // end namespace mux
