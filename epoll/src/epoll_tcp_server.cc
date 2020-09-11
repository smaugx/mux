#include "epoll/include/epoll_tcp_server.h"

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

EpollTcpServer::EpollTcpServer(const std::string& local_ip, uint16_t local_port)
    : local_ip_ { local_ip },
      local_port_ { local_port } {
}

EpollTcpServer::~EpollTcpServer() {
    Stop();
}

bool EpollTcpServer::Start() {
    if (CreateEpoll() < 0) {
        return false;
    }
    // create socket and bind
    int listenfd = CreateSocket();
    if (listenfd < 0) {
        return false;
    }
    int mr = MakeSocketNonBlock(listenfd);
    if (mr < 0) {
        return false;
    }

    int lr = Listen(listenfd);
    if (lr < 0) {
        return false;
    }
    handle_ = listenfd;

    auto sockptr = std::make_shared<Socket>(handle_, local_ip_, local_port_);
    int er = UpdateEpollEvents(efd_, EPOLL_CTL_ADD, handle_, EPOLLIN | EPOLLOUT | EPOLLET, sockptr->GetSocketImp());
    if (er < 0) {
        sockptr->Close();
        return false;
    }
    main_sock_ = sockptr;

    assert(!th_loop_);

    th_loop_ = std::make_shared<std::thread>(&EpollTcpServer::EpollLoop, this);
    if (!th_loop_) {
        MUX_ERROR("create thread for epollloop failed");
        return false;
    }
    th_loop_->detach();

    MUX_INFO("EpollTcpServer Start OK, local ip:{0} port:{1}", local_ip_, local_port_);
    return true;
}


bool EpollTcpServer::Stop() {
    loop_flag_ = false;
    main_sock_->Close();
    if (efd_ != -1) {
        ::close(efd_);
        efd_ = -1;
    }
    UnRegisterOnRecvCallback();
    accept_callback_ = nullptr;
    MUX_INFO("EpollTcpServer Stop OK");
}

int32_t EpollTcpServer::CreateEpoll() {
    int epollfd = epoll_create(1);
    if (epollfd < 0) {
        MUX_ERROR("epoll_create failed!");
        return -1;
    }
    efd_ = epollfd;
    return epollfd;
}

int32_t EpollTcpServer::CreateSocket() {
    int listenfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        MUX_ERROR("create socket failed!");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(local_port_);
    addr.sin_addr.s_addr  = inet_addr(local_ip_.c_str());

    int r = ::bind(listenfd, (struct sockaddr*)&addr, sizeof(struct sockaddr));
    if (r != 0) {
        MUX_ERROR("bind socket local_ip:{0} local_port:{1} failed", local_ip_, local_port_);
        ::close(listenfd);
        return -1;
    }
    MUX_INFO("create and bind socket:{0} success", listenfd);
    return listenfd;
}

int32_t EpollTcpServer::MakeSocketNonBlock(int32_t fd) {
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

int32_t EpollTcpServer::Listen(int32_t listenfd) {
    //int r = ::listen(listenfd, SOMAXCONN);
    int r = ::listen(listenfd, 3);
    if ( r < 0) {
        MUX_ERROR("listen failed");
        return -1;
    }
    return 0;
}

int32_t EpollTcpServer::UpdateEpollEvents(int efd, int op, int fd, int events, void* ptr) {
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

void EpollTcpServer::OnSocketAccept() {
    // epoll working on et mode, must read all coming data
    while (true) {
        struct sockaddr_in in_addr;
        socklen_t in_len = sizeof(in_addr);

        int cli_fd = accept(handle_, (struct sockaddr*)&in_addr, &in_len);
        if (cli_fd == -1) {
            if ( (errno == EAGAIN) || (errno == EWOULDBLOCK) ) {
                MUX_INFO("accept all coming connections success");
                break;
            } else {
                MUX_ERROR("accept error");
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
        std::string remote_ip(inet_ntoa(in_addr.sin_addr));
        uint16_t remote_port = in_addr.sin_port;
        MUX_DEBUG("accept connection from {0}:{1}", remote_ip, remote_port);
        fprintf(stdout, "accept connection from %s:%u\n", remote_ip.c_str(), remote_port);
        fflush(stdout);
        int mr = MakeSocketNonBlock(cli_fd);
        if (mr < 0) {
            ::close(cli_fd);
            continue;
        }

        auto sockptr = std::make_shared<Socket>(cli_fd, local_ip_, local_port_, remote_ip, remote_port);
        if (recv_callback_) {
            sockptr->RegisterOnRecvCallback(recv_callback_);
        }
        int er = UpdateEpollEvents(efd_, EPOLL_CTL_ADD, cli_fd, EPOLLIN | EPOLLOUT| EPOLLRDHUP | EPOLLET, sockptr->GetSocketImp());
        if (er < 0 ) {
            sockptr->Close();
            continue;
        }

        if (accept_callback_) {
            accept_callback_(sockptr);
            MUX_DEBUG("call accept callback for {0}:{1}", remote_ip, remote_port);
        }
    }
}

void EpollTcpServer::RegisterOnRecvCallback(callback_recv_t callback) {
    assert(!recv_callback_);
    recv_callback_ = callback;
    MUX_INFO("register recv callback");
}

void EpollTcpServer::UnRegisterOnRecvCallback() {
    assert(recv_callback_);
    recv_callback_ = nullptr;
    MUX_INFO("unregister recv callback");
}

void EpollTcpServer::RegisterOnAcceptCallback(callback_accept_t callback) {
    assert(!accept_callback_);
    accept_callback_ = callback;
    MUX_INFO("register accept callback");
}

// handle read events on fd
//void EpollTcpServer::OnSocketRead(int32_t fd) {
//    char read_buf[4096];
//    bzero(read_buf, sizeof(read_buf));
//    int n = -1;
//    while ( (n = ::read(fd, read_buf, sizeof(read_buf))) > 0) {
//        // callback for recv
//        MUX_DEBUG("recv_size:{0} from fd:{1}", n, fd);
//        std::string msg(read_buf, n);
//        PacketPtr packet = std::make_shared<Packet>(fd, msg);
//        if (recv_callback_) {
//            recv_callback_(packet);
//        } else {
//            MUX_WARN("no recv callback reigistered!");
//        }
//    }
//    if (n == -1) {
//        if (errno == EAGAIN || errno == EWOULDBLOCK) {
//            // read finished
//            MUX_DEBUG("read finished in fd:{0}", fd);
//            return;
//        }
//        // something goes wrong for this fd, should close it
//        ::close(fd);
//        MUX_ERROR("something goes wrong, will close this fd:{0}.", fd);
//        return;
//    }
//    if (n == 0) {
//        // this may happen when client close socket. EPOLLRDHUP usually handle this, but just make sure; should close this fd
//        ::close(fd);
//        MUX_ERROR("peer maybe closed, will close this fd:{0}.", fd);
//        return;
//    }
//}

// handle write events on fd (usually happens when sending big files)
//void EpollTcpServer::OnSocketWrite(int32_t fd) {
//    MUX_DEBUG("fd:{0} writeabled!", fd);
//}

//int32_t EpollTcpServer::SendData(const PacketPtr& packet) {
//    if (packet->fd_ <= 0) {
//        MUX_WARN("send failed, fd:{0} invalid", packet->fd_);
//        return -1;
//    }
//    int r = ::write(packet->fd_, packet->msg_.data(), packet->msg_.size());
//    if (r == -1) {
//        if (errno == EAGAIN || errno == EWOULDBLOCK) {
//            MUX_INFO("send {0} bytes finished", packet->msg_.size());
//            return -1;
//        }
//        // error happend
//        ::close(packet->fd_);
//        MUX_ERROR("write error, will close this fd:{0}", packet->fd_);
//        return -1;
//    }
//    MUX_DEBUG("write size:{0} in fd:{1} ok", r, packet->fd_);
//    return r;
//}

void EpollTcpServer::EpollLoop() {
    struct epoll_event* alive_events =  static_cast<epoll_event*>(calloc(kMaxEvents, sizeof(epoll_event)));
    if (!alive_events) {
        MUX_ERROR("calloc memory failed for epoll_events");
        return;
    }
    while (loop_flag_) {
        int num = epoll_wait(efd_, alive_events, kMaxEvents, kEpollWaitTime);

        for (int i = 0; i < num; ++i) {
            SocketCore* sock_core = static_cast<SocketCore*>(alive_events[i].data.ptr);
            int events = alive_events[i].events;

            if ( (events & EPOLLERR) || (events & EPOLLHUP) ) {
                MUX_DEBUG("epollerr or epollhup");
                // An error has occured on this fd, or the socket is not ready for reading (why were we notified then?).
                sock_core->notify_socket_event(ERR_EVENT);
            } else  if (events & EPOLLRDHUP) {
                MUX_DEBUG("epollrdhup");
                // Stream socket peer closed connection, or shut down writing half of connection.
                // more inportant, We still to handle disconnection when read()/recv() return 0 or -1 just to be sure.
                MUX_WARN("peer maybe closed, will close this fd:{0}", sock_core->fd_);
                // close fd and epoll will remove it
                sock_core->notify_socket_event(ERR_EVENT);
            } else if ( events & EPOLLIN ) {
                if (sock_core->fd_ == handle_) {
                    MUX_DEBUG("epollin accept");
                    // listen fd coming connections
                    OnSocketAccept();
                } else {
                    MUX_DEBUG("epollin read");
                    // other fd read event coming, meaning data coming
                    sock_core->notify_socket_event(READ_EVENT);
                }
            } else if ( events & EPOLLOUT ) {
                MUX_DEBUG("epollout");
                // write event for fd (not including listen-fd), meaning send buffer is available for big files
                sock_core->notify_socket_event(WRITE_EVENT);
            } else {
                MUX_WARN("unknow epoll event");
            }
        } // end for (int i = 0; ...

    } // end while (loop_flag_)
}

} // end namespace transport
} // end namespace mux
