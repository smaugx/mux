#pragma once


#include <memory>

#include "epoll/include/epoll_tcp_client.h"
#include "epoll/include/epoll_tcp_server.h"

namespace mux {

namespace transport {

class EventTriggerBase {
public:
    EventTriggerBase()                                         = default;
    EventTriggerBase(const EventTriggerBase& other)            = delete;
    EventTriggerBase& operator=(const EventTriggerBase& other) = delete;
    EventTriggerBase(EventTriggerBase&& other)                 = delete;
    EventTriggerBase& operator=(EventTriggerBase&& other)      = delete;

    virtual ~EventTriggerBase() = 0;
public:
    virtual bool Start() = 0;
    virtual bool Stop() = 0;
    virtual void RegisterOnRecvCallback(callback_recv_t callback) = 0;
    virtual void UnRegisterOnRecvCallback() = 0;
    virtual void RegisterOnAcceptCallback(callback_accept_t callback) = 0;
    virtual SocketPtr GetSocket() = 0;

};

class EventTriggerCli : public EventTriggerBase {
public:
    EventTriggerCli()                                        = default;
    EventTriggerCli(const EventTriggerCli& other)            = delete;
    EventTriggerCli& operator=(const EventTriggerCli& other) = delete;
    EventTriggerCli(EventTriggerCli&& other)                 = delete;
    EventTriggerCli& operator=(EventTriggerCli&& other)      = delete;

public:
    EventTriggerCli(const std::string& server_ip, uint16_t server_port);
    ~EventTriggerCli() override;

public:
    bool Start() override;
    bool Stop() override;
    void RegisterOnRecvCallback(callback_recv_t callback) override;
    void UnRegisterOnRecvCallback() override;
    void RegisterOnAcceptCallback(callback_accept_t callback) override;
    SocketPtr GetSocket() override;

private:
    std::shared_ptr<EpollTcpClient> reactor_ { nullptr }; // using epoll or anything else (like select, poll, iocp...)
};

class EventTriggerSvr : public EventTriggerBase {
public:
    EventTriggerSvr()                                        = default;
    EventTriggerSvr(const EventTriggerSvr& other)            = delete;
    EventTriggerSvr& operator=(const EventTriggerSvr& other) = delete;
    EventTriggerSvr(EventTriggerSvr&& other)                 = delete;
    EventTriggerSvr& operator=(EventTriggerSvr&& other)      = delete;

public:
    EventTriggerSvr(const std::string& local_ip, uint16_t local_port);
    ~EventTriggerSvr() override;

public:
    bool Start() override;
    bool Stop() override;
    void RegisterOnRecvCallback(callback_recv_t callback) override;
    void UnRegisterOnRecvCallback() override;
    void RegisterOnAcceptCallback(callback_accept_t callback) override;
    SocketPtr GetSocket() override;

private:
    std::shared_ptr<EpollTcpServer> reactor_ { nullptr }; // using epoll or anything else (like select, poll, iocp...)
};

typedef std::shared_ptr<EventTriggerBase> EventTriggerBasePtr;
typedef std::shared_ptr<EventTriggerCli>  EventTriggerCliPtr;
typedef std::shared_ptr<EventTriggerSvr>  EventTriggerSvrPtr;



} // end namespace transport

} // end namespace mux
