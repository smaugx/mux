#include "epoll_tcp_server.h"

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
    int listenfd = CreateAndBind();
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
    std::cout << "EpollTcpServer Init success!" << std::endl;
    handle_ = listenfd;

    int er = UpdateEpollEvents(efd_, EPOLL_CTL_ADD, handle_, EPOLLIN | EPOLLET);
    if (er < 0) {
        ::close(handle_);
        return false;
    }

    assert(!th_loop_);

    th_loop_ = std::make_shared<std::thread>(std::bind(&EpollTcpServer::EpollLoop, this));
    if (!th_loop_) {
        return false;
    }

    return true;
}


bool EpollTcpServer::Stop() {
    loop_flag_ = false;
    ::close(handle_);
    ::close(efd_);
    std::cout << "stop epoll!" << std::endl;
    UnRegisterOnRecvCallback();
}

int32_t EpollTcpServer::CreateEpoll() {
    int epollfd = epoll_create(1);
    if (epollfd < 0) {
        std::cout << "epoll_create failed!" << std::endl;
        return -1;
    }
    efd_ = epollfd;
    return epollfd;
}

int32_t EpollTcpServer::CreateAndBind() {
    int listenfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        std::cout << "create socket " << local_ip_ << ":" << local_port_ << " failed!" << std::endl;
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(local_port_);
    addr.sin_addr.s_addr  = inet_addr(local_ip_.c_str());

    int r = ::bind(listenfd, (struct sockaddr*)&addr, sizeof(struct sockaddr));
    if (r != 0) {
        std::cout << "bind socket " << local_ip_ << ":" << local_port_ << " failed!" << std::endl;
        ::close(listenfd);
        return -1;
    }
    std::cout << "create and bind socket " << local_ip_ << ":" << local_port_ << " success!" << std::endl;
    return listenfd;
}

int32_t EpollTcpServer::MakeSocketNonBlock(int32_t fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        std::cout << "fcntl failed!" << std::endl;
        return -1;
    }
    int r = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    if (r < 0) {
        std::cout << "fcntl failed!" << std::endl;
        return -1;
    }
    return 0;
}

int32_t EpollTcpServer::Listen(int32_t listenfd) {
    int r = ::listen(listenfd, SOMAXCONN);
    if ( r < 0) {
        std::cout << "listen failed!" << std::endl;
        return -1;
    }
    return 0;
}

int32_t EpollTcpServer::UpdateEpollEvents(int efd, int op, int fd, int events) {
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = events;
    ev.data.fd = fd;
    fprintf(stdout,"%s fd %d events read %d write %d\n", op == EPOLL_CTL_MOD ? "mod" : "add", fd, ev.events & EPOLLIN, ev.events & EPOLLOUT);
    int r = epoll_ctl(efd, op, fd, &ev);
    if (r < 0) {
        std::cout << "epoll_ctl failed!" << std::endl;
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
                std::cout << "accept all coming connections!" << std::endl;
                break;
            } else {
                std::cout << "accept error!" << std::endl;
                continue;
            }
        }

        sockaddr_in peer;
        socklen_t p_len = sizeof(peer);
        int r = getpeername(cli_fd, (struct sockaddr*)&peer, &p_len);
        if (r < 0) {
            std::cout << "getpeername error!" << std::endl;
            continue;
        }
        std::cout << "accpet connection from " << inet_ntoa(in_addr.sin_addr) << std::endl;
        int mr = MakeSocketNonBlock(cli_fd);
        if (mr < 0) {
            ::close(cli_fd);
            continue;
        }

        int er = UpdateEpollEvents(efd_, EPOLL_CTL_ADD, cli_fd, EPOLLIN | EPOLLRDHUP | EPOLLET);
        if (er < 0 ) {
            ::close(cli_fd);
            continue;
        }
    }
}

void EpollTcpServer::RegisterOnRecvCallback(callback_recv_t callback) {
    assert(!recv_callback_);
    recv_callback_ = callback;
}

void EpollTcpServer::UnRegisterOnRecvCallback() {
    assert(recv_callback_);
    recv_callback_ = nullptr;
}

// handle read events on fd
void EpollTcpServer::OnSocketRead(int32_t fd) {
    char read_buf[4096];
    bzero(read_buf, sizeof(read_buf));
    int n = -1;
    while ( (n = ::read(fd, read_buf, sizeof(read_buf))) > 0) {
        // callback for recv
        std::cout << "fd: " << fd <<  " recv: " << read_buf << std::endl;
        std::string msg(read_buf, n);
        SocketDataPtr data = std::make_shared<SocketData>(fd, msg);
        if (recv_callback_) {
            recv_callback_(data);
        }
    }
    if (n == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // read finished
            return;
        }
        // something goes wrong for this fd, should close it
        ::close(fd);
        return;
    }
    if (n == 0) {
        // this may happen when client close socket. EPOLLRDHUP usually handle this, but just make sure; should close this fd
        ::close(fd);
        return;
    }
}

// handle write events on fd (usually happens when sending big files)
void EpollTcpServer::OnSocketWrite(int32_t fd) {
    std::cout << "fd: " << fd << " writeable!" << std::endl;
}

int32_t EpollTcpServer::SendData(const SocketDataPtr& data) {
    if (data->fd_ <= 0) {
        return -1;
    }
    int r = ::write(data->fd_, data->msg_.data(), data->msg_.size());
    if (r == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return -1;
        }
        // error happend
        ::close(data->fd_);
        std::cout << "fd: " << data->fd_ << " write error, close it!" << std::endl;
        return -1;
    }
    std::cout << "fd: " << data->fd_ << " write size: " << r << " ok!" << std::endl;
    return r;
}

void EpollTcpServer::EpollLoop() {
    struct epoll_event* alive_events =  static_cast<epoll_event*>(calloc(kMaxEvents, sizeof(epoll_event)));
    if (!alive_events) {
        std::cout << "calloc memory failed for epoll_events!" << std::endl;
        return;
    }
    while (loop_flag_) {
        int num = epoll_wait(efd_, alive_events, kMaxEvents, kEpollWaitTime);

        for (int i = 0; i < num; ++i) {
            int fd = alive_events[i].data.fd;
            int events = alive_events[i].events;

            if ( (events & EPOLLERR) || (events & EPOLLHUP) ) {
                std::cout << "epoll_wait error!" << std::endl;
                // An error has occured on this fd, or the socket is not ready for reading (why were we notified then?).
                ::close(fd);
            } else  if (events & EPOLLRDHUP) {
                // Stream socket peer closed connection, or shut down writing half of connection.
                // more inportant, We still to handle disconnection when read()/recv() return 0 or -1 just to be sure.
                std::cout << "fd:" << fd << " closed vis EPOLLRDHUP!" << std::endl;
                // close fd and epoll will remove it
                ::close(fd);
            } else if ( events & EPOLLIN ) {
                if (fd == handle_) {
                    // listen fd coming connections
                    OnSocketAccept();
                } else {
                    // other fd read event coming, meaning data coming
                    OnSocketRead(fd);
                }
            } else if ( events & EPOLLOUT ) {
                // write event for fd (not including listen-fd), meaning send buffer is available for big files
                OnSocketWrite(fd);
            } else {
                std::cout << "unknow epoll event!" << std::endl;
            }
        } // end for (int i = 0; ...

    } // end while (loop_flag_)
}

} // end namespace transport
} // end namespace mux
