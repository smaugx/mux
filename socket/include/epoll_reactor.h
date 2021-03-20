#pragma once

#include <sys/epoll.h>
#include <sys/socket.h>

#include <string>
#include <memory>
#include <thread>

#include "mbase/include/runnable.h"
#include "socket/include/socket_imp.h"

namespace mux {

namespace transport {

class EpollReactor : public RunEntity, public std::enable_shared_from_this<EpollReactor> {
public:
    EpollReactor(const EpollReactor& other)            = delete;
    EpollReactor& operator=(const EpollReactor& other) = delete;
    EpollReactor(EpollReactor&& other)                 = delete;
    EpollReactor& operator=(EpollReactor&& other)      = delete;

    EpollReactor();
    explicit EpollReactor(uint32_t eid);
    virtual ~EpollReactor();

public:
    bool Start() override;
    bool Stop() override;

public:
    void RegisterOnAcceptCallback(callback_accept_t callback);
    int32_t RegisterDescriptor(void* ptr, int events = EPOLLIN | EPOLLOUT| EPOLLRDHUP | EPOLLET);
    void RegisterOnSocketErrCallback(callback_sockerr_t callback);

protected:
    int32_t CreateEpoll();
    int32_t UpdateEpollEvents(int efd, int op, int fd, int events, void* ptr);
    void EpollLoop();
    int32_t MakeSocketNonBlock(int32_t fd);

    virtual void OnSocketAccept(void* ptr);
    virtual void OnSocketRead(void* ptr);
    virtual void OnSocketWrite(void* ptr);
    virtual void OnSocketError(void* ptr);


private:
    uint32_t eid_ { 0 }; // id of this epoll, begin from 0
    int32_t efd_ { -1 }; // epoll fd
    std::shared_ptr<std::thread> th_loop_ { nullptr };
    bool shutdown_flag_ { true };
    callback_accept_t accept_callback_ { nullptr };
    callback_sockerr_t sockerr_callback_ { nullptr };
};

typedef std::shared_ptr<EpollReactor> EpollReactorPtr;

}

}
