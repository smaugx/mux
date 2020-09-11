#include "epoll/include/epoll_tcp_client.h"

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
    handle_ = cli_fd;
    lr = MakeSocketNonBlock(handle_);
    if (lr < 0) {
        return false;
    }

    auto sockptr = std::make_shared<Socket>(cli_fd, local_ip_, local_port_, server_ip_, server_port_);
    int er = UpdateEpollEvents(efd_, EPOLL_CTL_ADD, handle_, EPOLLIN | EPOLLOUT | EPOLLET, sockptr->GetSocketImp());
    if (er < 0) {
        sockptr->Close();
        return false;
    }
    main_sock_ = sockptr;

    assert(!th_loop_);

    th_loop_ = std::make_shared<std::thread>(&EpollTcpClient::EpollLoop, this);
    if (!th_loop_) {
        return false;
    }
    th_loop_->detach();

    MUX_INFO("EpollTcpClient Start OK, target ip:{0} port:{1} fd:{2}", server_ip_, server_port_, handle_);

    return true;
}


bool EpollTcpClient::Stop() {
    loop_flag_ = false;
    main_sock_->Close();
    if (efd_ != -1) {
        ::close(efd_);
        efd_ = -1;
    }
    UnRegisterOnRecvCallback();
    MUX_INFO("EpollTcpClient Stop OK");
}

int32_t EpollTcpClient::CreateEpoll() {
    int epollfd = epoll_create(1);
    if (epollfd < 0) {
        MUX_ERROR("epoll_create failed!");
        return -1;
    }
    efd_ = epollfd;
    return epollfd;
}

int32_t EpollTcpClient::CreateSocket() {
    int cli_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (cli_fd < 0) {
        MUX_ERROR("create socket failed!");
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
        MUX_ERROR("connect {0} {1} failed!", server_ip_, server_port_);
        return -1;
    }

    struct sockaddr_in local_addr;
    int la_len = sizeof(local_addr);
    if (getsockname(cli_fd,  (struct sockaddr *)&local_addr, (socklen_t*)&la_len) == -1) {
        MUX_ERROR("getsockname failed");
        return -1;
    }
    local_ip_ = inet_ntoa(local_addr.sin_addr);
    local_port_ = ntohs(local_addr.sin_port);

    return 0;
}

int32_t EpollTcpClient::MakeSocketNonBlock(int32_t fd) {
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

int32_t EpollTcpClient::UpdateEpollEvents(int efd, int op, int fd, int events, void* ptr) {
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

void EpollTcpClient::RegisterOnRecvCallback(callback_recv_t callback) {
    assert(!recv_callback_);
    recv_callback_ = callback;
    MUX_INFO("register recv callback");
}

void EpollTcpClient::UnRegisterOnRecvCallback() {
    assert(recv_callback_);
    recv_callback_ = nullptr;
    MUX_INFO("unregister recv callback");
}

void EpollTcpClient::RegisterOnAcceptCallback(callback_accept_t callback) {
    // do nothing for client
    return;
}

//// handle read events on fd
//void EpollTcpClient::OnSocketRead(int32_t fd) {
//    char read_buf[4096];
//    bzero(read_buf, sizeof(read_buf));
//    int n = -1;
//    while ( (n = ::read(fd, read_buf, sizeof(read_buf))) > 0) {
//        // callback for recv
//        MUX_DEBUG("recv_size:{0} from fd:{1}", n, fd);
//        std::string msg(read_buf, n);
//        PacketPtr packet = std::make_shared<Packet>(fd, msg);
//        if (recv_callback_) {
//            MUX_DEBUG("recv callback");
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
//    // should not come here
//    MUX_ERROR("invalid read ret = {0}", n);
//}

//// handle write events on fd (usually happens when sending big files)
//void EpollTcpClient::OnSocketWrite(int32_t fd) {
//    MUX_DEBUG("fd:{0} writeabled!", fd);
//}

//int32_t EpollTcpClient::SendData(const PacketPtr& packet) {
//    int32_t fd = packet->fd_;
//    if (fd <= 0) {
//        fd = handle_; // specially for client
//    }
//    if (fd <= 0) {
//        MUX_WARN("send failed, fd:{0} invalid", fd);
//        return -1;
//    }
//    int r = ::write(fd, packet->msg_.data(), packet->msg_.size());
//    if (r == -1) {
//        if (errno == EAGAIN || errno == EWOULDBLOCK) {
//            MUX_INFO("send {0} bytes finished", packet->msg_.size());
//            return -1;
//        }
//        // error happend
//        ::close(fd);
//        MUX_ERROR("write error, will close this fd:{0}", fd);
//        return -1;
//    }
//    MUX_DEBUG("write size:{0} in fd:{1} ok", r, fd);
//    return r;
//}

void EpollTcpClient::EpollLoop() {
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
                MUX_ERROR("epoll_wait error, will close fd:{0}", sock_core->fd_);
                // An error has occured on this fd, or the socket is not ready for reading (why were we notified then?).
                sock_core->notify_socket_event(ERR_EVENT);
            } else  if (events & EPOLLRDHUP) {
                // Stream socket peer closed connection, or shut down writing half of connection.
                // more inportant, We still to handle disconnection when read()/recv() return 0 or -1 just to be sure.
                MUX_WARN("peer maybe closed, will close this fd:{0}", sock_core->fd_);
                // close fd and epoll will remove it
                sock_core->notify_socket_event(ERR_EVENT);
            } else if ( events & EPOLLIN ) {
                // other fd read event coming, meaning data coming
                sock_core->notify_socket_event(READ_EVENT);
            } else if ( events & EPOLLOUT ) {
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
