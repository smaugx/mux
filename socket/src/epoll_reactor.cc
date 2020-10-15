#include "socket/include/epoll_reactor.h"

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
#include <ctime>

#include <iostream>

#include "mbase/include/mux_log.h"

namespace mux {

namespace transport {

static const uint32_t kEpollWaitTime = 10; // 10 ms
static const uint32_t kMaxEvents = 100;

std::string time_point_to_string(std::chrono::system_clock::time_point &tp)
{
    using namespace std;
    using namespace std::chrono;

    auto ttime_t = system_clock::to_time_t(tp);
    auto tp_sec = system_clock::from_time_t(ttime_t);
    milliseconds ms = duration_cast<milliseconds>(tp - tp_sec);

    std::tm * ttm = localtime(&ttime_t);

    char date_time_format[] = "%Y.%m.%d %H:%M:%S";

    char time_str[] = "yyyy.mm.dd.HH-MM.SS.fff";

    strftime(time_str, strlen(time_str), date_time_format, ttm);

    string result(time_str);
    result.append(".");
    result.append(to_string(ms.count()));

    return result;
}


EpollReactor::EpollReactor() {
    int r = CreateEpoll();
    assert(r >= 0);
}

EpollReactor::EpollReactor(uint32_t eid)
    : eid_(eid) {
    int r = CreateEpoll();
    assert(r>=0);
}

EpollReactor::~EpollReactor() {
    Stop();
}

bool EpollReactor::Start() {
    shutdown_flag_ = false;
    MUX_DEBUG("set epollreactor shutdown_flag {0}", shutdown_flag_);

    assert(!th_loop_);
    auto self(shared_from_this());
    th_loop_ = std::make_shared<std::thread>(&EpollReactor::EpollLoop, self);
    if (!th_loop_) {
        MUX_ERROR("create thread for epollloop failed");
        return false;
    }
    th_loop_->detach();

    MUX_INFO("EpollReactor Start OK");
    return true;
}


bool EpollReactor::Stop() {
    if (shutdown_flag_) {
        return true;
    }
    shutdown_flag_ = true;
    ::close(efd_);
    MUX_INFO("EpollReactor Stop OK");
    return true;
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


int32_t EpollReactor::MakeSocketNonBlock(int32_t fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        MUX_ERROR("fcntl failed in fd:{0}", fd);
        return -1;
    }
    int r = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    if (r < 0) {
        MUX_ERROR("fcntl failed in fd:{0}", fd);
        return -1;
    }
    return 0;
}

int32_t EpollReactor::RegisterDescriptor(void* ptr, int events) {
    SocketBase* sock = static_cast<SocketBase*>(ptr);
    int fd = sock->GetDescriptor();
    MUX_INFO("RegisterDescriptor fd:{0}", fd);
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
    assert(!accept_callback_);
    accept_callback_ = callback;
}

// only for server, client will never generate accept-event
void EpollReactor::OnSocketAccept(void* ptr) {
    SocketBase* sock = static_cast<SocketBase*>(ptr);
    int32_t handle = sock->GetDescriptor();
    uint32_t accept_connections = 0;
    // epoll working on et mode, must read all coming data
    while (true) {
        struct sockaddr_in in_addr;
        socklen_t in_len = sizeof(in_addr);

        int cli_fd = accept(handle, (struct sockaddr*)&in_addr, &in_len);
        if (cli_fd == -1) {
            if ( (errno == EAGAIN) || (errno == EWOULDBLOCK) ) {
                if (accept_connections > 0) {
                    MUX_INFO("eid:{0} accept all coming connections success", eid_);
                } else {
                    MUX_WARN("eid:{0} accept error, thundering herd happened", eid_);
                }
                break;
            } else {
                MUX_ERROR("eid:{0} accept error", eid_);
                continue;
            }
        }

        sockaddr_in peer;
        socklen_t p_len = sizeof(peer);
        int r = getpeername(cli_fd, (struct sockaddr*)&peer, &p_len);
        if (r < 0) {
            MUX_WARN("getpeername error in fd:{0}", cli_fd);
            continue;
        }
        auto now_time = std::chrono::system_clock::now();
        auto now_time_str = time_point_to_string(now_time);
        std::string remote_ip(inet_ntoa(in_addr.sin_addr));
        uint16_t remote_port = in_addr.sin_port;
        MUX_DEBUG("eid:{0} accept connection from {1}:{2} time:{3}",eid_, remote_ip, remote_port, now_time_str);
        fprintf(stdout, "accept connection from %s:%u\n", remote_ip.c_str(), remote_port);
        fflush(stdout);
        int mr = MakeSocketNonBlock(cli_fd);
        if (mr < 0) {
            ::close(cli_fd);
            continue;
        }

        if (!accept_callback_) {
            MUX_WARN("accept callback empty, refuse create new connection");
            continue;
        }

        BasicSocket* new_sock = accept_callback_(cli_fd, remote_ip, remote_port);
        if (!new_sock) {
            MUX_ERROR("error create muxsocket");
            ::close(cli_fd);
            continue;
        }
        int rd = RegisterDescriptor((void*)new_sock);
        if (rd < 0 ) {
            new_sock->Close();
        }
        accept_connections += 1;
    } // end while(true)
}

// handle read events on fd
void EpollReactor::OnSocketRead(void* ptr) {
    BasicSocket* sock = static_cast<BasicSocket*>(ptr);
    sock->HandleRead();
}

// handle write events on fd (usually happens when sending big files)
void EpollReactor::OnSocketWrite(void* ptr) {
    BasicSocket* sock = static_cast<BasicSocket*>(ptr);
    sock->HandleWrite();
}

void EpollReactor::OnSocketError(void* ptr) {
    BasicSocket* sock = static_cast<BasicSocket*>(ptr);
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
                MUX_DEBUG("eid:{0} epollerr or epollhup", eid_);
                // An error has occured on this fd, or the socket is not ready for reading (why were we notified then?).
                OnSocketError(ptr);
            } else  if (events & EPOLLRDHUP) {
                MUX_DEBUG("eid:{0} epollrdhup", eid_);
                // Stream socket peer closed connection, or shut down writing half of connection.
                // more inportant, We still to handle disconnection when read()/recv() return 0 or -1 just to be sure.
                // close fd and epoll will remove it
                OnSocketError(ptr);
            } else if ( events & EPOLLIN ) {
                MUX_DEBUG("eid:{0} epollin",eid_);
                SocketBase* sock = static_cast<SocketBase*>(ptr);
                if (sock->CheckListener()) {
                    // listen fd coming connections
                    OnSocketAccept(ptr);
                } else {
                    // other fd read event coming, meaning data coming
                    OnSocketRead(ptr);
                }
                sock = nullptr;
            } else if ( events & EPOLLOUT ) {
                MUX_DEBUG("eid:{0} epollout", eid_);
                // write event for fd (not including listen-fd), meaning send buffer is available for big files
                OnSocketWrite(ptr);
            } else {
                MUX_WARN("eid:{0} unknow epoll event", eid_);
            }
        } // end for (int i = 0; ...

    } // end while (!shutdown_flag_)

    MUX_INFO("eid:{0} shutdown_flag:{1} will shutdown epoll_reactor", eid_, shutdown_flag_);
    free(alive_events);
}

} // end namespace transport
} // end namespace mux
