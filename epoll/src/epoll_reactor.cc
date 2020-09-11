#include "epoll/include/epoll_reactor.h"

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

static const uint32_t kEpollWaitTime = 10; // 10 ms
static const uint32_t kMaxEvents = 100;

EpollReactor::EpollReactor() {}

EpollReactor::EpollReactor(int32_t handle)
    : handle_(handle) {}

EpollReactor::EpollReactor(uint32_t eid, int32_t handle)
    : eid_(eid),
      handle_(handle) {}

EpollReactor::~EpollReactor() {
    Stop();
}

bool EpollReactor::Start() {
    if (CreateEpoll() < 0) {
        return false;
    }
    shutdown_flag_ = false;
    MUX_DEBUG("set epollreactor shutdown_flag {0}", shutdown_flag_);

    if (handle_ != -1) {
        SocketBase* sock = new BasicSocket(handle_);
        RegisterDescriptor((void*)&sock, handle_);
        MUX_INFO("register listen handle:{0} to epollreactor:{1}", handle_, eid_);
    }

    assert(!th_loop_);

    th_loop_ = std::make_shared<std::thread>(&EpollReactor::EpollLoop, this);
    if (!th_loop_) {
        MUX_ERROR("create thread for epollloop failed");
        return false;
    }
    th_loop_->detach();

    MUX_INFO("EpollReactor Start OK");
    return true;
}


bool EpollReactor::Stop() {
    shutdown_flag_ = true;
    ::close(efd_);
    MUX_INFO("EpollReactor Stop OK");
}

int32_t EpollReactor::CreateEpoll() {
    int epollfd = epoll_create(1);
    if (epollfd < 0) {
        MUX_ERROR("epoll_create failed!");
        return -1;
    }
    efd_ = epollfd;
    return epollfd;
}

int32_t EpollReactor::RegisterDescriptor(void* ptr, int fd, int events) {
    return UpdateEpollEvents(efd_, EPOLL_CTL_ADD, fd, events, ptr);
}

int32_t EpollReactor::UpdateEpollEvents(int efd, int op, int fd, int events, void* ptr) {
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = events;
    ev.data.ptr = ptr;
    int r = epoll_ctl(efd, op, fd, &ev);
    if (r < 0) {
        MUX_ERROR("epoll_ctl failed for fd:{0}", fd);
        return -1;
    }
    return 0;
}


void EpollReactor::RegisterOnAcceptCallback(callback_accept_t callback) {
    if (handle_ == -1) {
        MUX_INFO("not listen descriptor, ignore register accept callback");
        return;
    }
    assert(!accept_callback_);
    accept_callback_ = callback;
}

void EpollReactor::OnSocketAccept() {
    if (!accept_callback_) {
        MUX_WARN("accept callback invalid");
        return;
    }

    accept_callback_(eid_);
}

// handle read events on fd
void EpollReactor::OnSocketRead(void* ptr) {
    SocketBase* sock = dynamic_cast<SocketBase*>(ptr);
    sock->HandleRead();
}

// handle write events on fd (usually happens when sending big files)
void EpollReactor::OnSocketWrite(void* ptr) {
    SocketBase* sock = dynamic_cast<SocketBase*>(ptr);
    sock->HandleWrite();
}

void EpollReactor::OnSocketError(void* ptr) {
    SocketBase* sock = dynamic_cast<SocketBase*>(ptr);
    sock->HandleError();
}

void EpollReactor::EpollLoop() {
    struct epoll_event* alive_events =  static_cast<epoll_event*>(calloc(kMaxEvents, sizeof(epoll_event)));
    if (!alive_events) {
        MUX_ERROR("calloc memory failed for epoll_events");
        return;
    }
    while (!shutdown_flag_) {
        int num = epoll_wait(efd_, alive_events, kMaxEvents, kEpollWaitTime);

        for (int i = 0; i < num; ++i) {
            void* ptr = alive_events[i].data.ptr;
            int events = alive_events[i].events;

            if ( (events & EPOLLERR) || (events & EPOLLHUP) ) {
                MUX_DEBUG("epollerr or epollhup");
                // An error has occured on this fd, or the socket is not ready for reading (why were we notified then?).
                OnSocketError(ptr);
            } else  if (events & EPOLLRDHUP) {
                MUX_DEBUG("epollrdhup");
                // Stream socket peer closed connection, or shut down writing half of connection.
                // more inportant, We still to handle disconnection when read()/recv() return 0 or -1 just to be sure.
                // close fd and epoll will remove it
                OnSocketError(ptr);
            } else if ( events & EPOLLIN ) {
                MUX_DEBUG("epollin");
                SocketBase* sock = dynamic_cast<SocketBase*>(ptr);
                int32_t fd = sock->GetDescriptor();
                if (handle_ != -1 && fd == handle_) {
                    // listen fd coming connections
                    OnSocketAccept();
                } else {
                    // other fd read event coming, meaning data coming
                    OnSocketRead(ptr);
                }
            } else if ( events & EPOLLOUT ) {
                MUX_DEBUG("epollout");
                // write event for fd (not including listen-fd), meaning send buffer is available for big files
                OnSocketWrite(ptr);
            } else {
                MUX_WARN("unknow epoll event");
            }
        } // end for (int i = 0; ...

    } // end while (!shutdown_flag_)
    delete []alive_events;
}

} // end namespace transport
} // end namespace mux
