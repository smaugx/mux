#pragma once

#include <string>
#include <memory>
#include <thread>

#include "epoll/include/socket_imp.h"

namespace mux {

namespace transport {

class EpollReactor {
public:
    EpollReactor(const EpollReactor& other)            = delete;
    EpollReactor& operator=(const EpollReactor& other) = delete;
    EpollReactor(EpollReactor&& other)                 = delete;
    EpollReactor& operator=(EpollReactor&& other)      = delete;

    EpollReactor();
    explicit EpollReactor(int32_t handle);
    EpollReactor(uint32_t eid, int32_t handle);
    virtual ~EpollReactor();

public:
    bool Start();
    void RegisterOnAcceptCallback(callback_accept_t callback);
    int32_t RegisterDescriptor(void* ptr, int fd, int events = EPOLLIN | EPOLLOUT| EPOLLRDHUP | EPOLLET);
    bool Stop();

protected:
    int32_t CreateEpoll();
    int32_t UpdateEpollEvents(int efd, int op, int fd, int events, void* ptr);
    void EpollLoop();

    void OnSocketAccept();
    void OnSocketRead(void* ptr);
    void OnSocketWrite(void* ptr);
    void OnSocketError(void* ptr);


private:
    uint32_t eid_ { 0 }; // id of this epoll, begin from 0
    int32_t handle_ { -1 }; // listenfd
    int32_t efd_ { -1 }; // epoll fd
    std::shared_ptr<std::thread> th_loop_ { nullptr };
    bool shutdown_flag_ { true };
    callback_accept_t accept_callback_ { nullptr };
};

typedef std::shared_ptr<EpollReactor> EpollReactorPtr;

}

}