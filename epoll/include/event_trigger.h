#pragma once

#include "epoll/include/epoll_reactor.h"

namespace mux {

namespace transport {

class EventTriggerBase {
public:
    EventTriggerBase()                                         = default;
    EventTriggerBase(const EventTriggerBase& other)            = delete;
    EventTriggerBase& operator=(const EventTriggerBase& other) = delete;
    EventTriggerBase(EventTriggerBase&& other)                 = delete;
    EventTriggerBase& operator=(EventTriggerBase&& other)      = delete;

    virtual ~EventTriggerBase() {}
public:
};

class EventTrigger : public EventTriggerBase {
public:
    EventTrigger(const EventTrigger& other)            = delete;
    EventTrigger& operator=(const EventTrigger& other) = delete;
    EventTrigger(EventTrigger&& other)                 = delete;
    EventTrigger& operator=(EventTrigger&& other)      = delete;

public:
    EventTrigger();
    EventTrigger(int32_t handle, int ep_num);
    ~EventTrigger() override;

public:
    bool Start() override;
    void RegisterOnAcceptCallback(callback_accept_t callback) override;
    bool Stop() override;

private:
    int32_t handle_ { -1 };
    int ep_num_ { 1 };
    std::vector<EpollReactorPtr> reactor_vec_;
};


typedef std::shared_ptr<EventTriggerBase> EventTriggerBasePtr;
typedef std::shared_ptr<EventTrigger>  EventTriggerPtr;


} // end namespace transport

} // end namespace mux
