#include "epoll_tcp_client.h"

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

EpollTcpClient::EpollTcpClient(const std::string& server_ip, uint16_t server_port)
    : server_ip_ { server_ip },
      server_port_ { server_port } {
}

EpollTcpClient::~EpollTcpClient() {
    Stop();
}

bool EpollTcpClient::Start() {
    if (CreateEpoll() < 0) {
        return false;
    }
    // create socket and bind
    int cli_fd  = CreateSocket();
    if (cli_fd < 0) {
        return false;
    }

    int lr = Connect(cli_fd);
    if (lr < 0) {
        return false;
    }
    std::cout << "EpollTcpClient Init success!" << std::endl;
    handle_ = cli_fd;

    int er = UpdateEpollEvents(efd_, EPOLL_CTL_ADD, handle_, EPOLLIN | EPOLLET);
    if (er < 0) {
        ::close(handle_);
        return false;
    }

    assert(!th_loop_);

    th_loop_ = std::make_shared<std::thread>(&EpollTcpClient::EpollLoop, this);
    if (!th_loop_) {
        return false;
    }
    th_loop_->detach();

    return true;
}


bool EpollTcpClient::Stop() {
    loop_flag_ = false;
    ::close(handle_);
    ::close(efd_);
    std::cout << "stop epoll!" << std::endl;
    UnRegisterOnRecvCallback();
}

int32_t EpollTcpClient::CreateEpoll() {
    int epollfd = epoll_create(1);
    if (epollfd < 0) {
        std::cout << "epoll_create failed!" << std::endl;
        return -1;
    }
    efd_ = epollfd;
    return epollfd;
}

int32_t EpollTcpClient::CreateSocket() {
    int cli_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (cli_fd < 0) {
        std::cout << "create socket failed!" << std::endl;
        return -1;
    }

    return cli_fd;
}

int32_t EpollTcpClient::Connect(int32_t cli_fd) {
    struct sockaddr_in addr;  // server info
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(server_port_);
    addr.sin_addr.s_addr  = inet_addr(server_ip_.c_str());

    int r = ::connect(cli_fd, (struct sockaddr*)&addr, sizeof(addr));
    if ( r < 0) {
        std::cout << "connect failed! r=" << r << " errno:" << errno << std::endl;
        return -1;
    }
    return 0;
}

int32_t EpollTcpClient::UpdateEpollEvents(int efd, int op, int fd, int events) {
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

void EpollTcpClient::RegisterOnRecvCallback(callback_recv_t callback) {
    assert(!recv_callback_);
    recv_callback_ = callback;
}

void EpollTcpClient::UnRegisterOnRecvCallback() {
    assert(recv_callback_);
    recv_callback_ = nullptr;
}

// handle read events on fd
void EpollTcpClient::OnSocketRead(int32_t fd) {
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
void EpollTcpClient::OnSocketWrite(int32_t fd) {
    std::cout << "fd: " << fd << " writeable!" << std::endl;
}

int32_t EpollTcpClient::SendData(const SocketDataPtr& data) {
    int32_t fd = data->fd_;
    if (fd <= 0) {
        fd = handle_; // specially for client
    }
    if (fd <= 0) {
        return -1;
    }
    int r = ::write(fd, data->msg_.data(), data->msg_.size());
    if (r == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return -1;
        }
        // error happend
        ::close(fd);
        std::cout << "fd: " << fd << " write error, close it!" << std::endl;
        return -1;
    }
    std::cout << "fd: " << fd << " write size: " << r << " ok!" << std::endl;
    return r;
}

void EpollTcpClient::EpollLoop() {
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
                // other fd read event coming, meaning data coming
                OnSocketRead(fd);
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
