#pragma once

#include <vector>

#include "socket/include/epoll_reactor.h"
#include "mbase/include/runnable.h"

namespace mux {

namespace transport {

class EventTrigger : public RunEntity {
public:
    EventTrigger(const EventTrigger& other)            = delete;
    EventTrigger& operator=(const EventTrigger& other) = delete;
    EventTrigger(EventTrigger&& other)                 = delete;
    EventTrigger& operator=(EventTrigger&& other)      = delete;

public:
    EventTrigger();
    explicit EventTrigger(int ep_num);
    virtual ~EventTrigger();

public:
    bool Start() override;
    bool Stop() override;

public:
    void RegisterOnAcceptCallback(callback_accept_t callback);
    int32_t RegisterDescriptor(void* ptr, int events = EPOLLIN | EPOLLOUT| EPOLLRDHUP | EPOLLET);

private:
    int ep_num_ { 1 };
    std::vector<EpollReactorPtr> reactor_vec_;
};


typedef std::shared_ptr<EventTrigger>  EventTriggerPtr;


} // end namespace transport

} // end namespace mux
