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

TcpAcceptor::TcpAcceptor(const std::string& local_ip, uint16_t local_port)
    : local_ip_ { local_ip },
      local_port_ { local_port } {
}

TcpAcceptor::~TcpAcceptor() {
    Stop();
}

bool TcpAcceptor::Start() {
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

    MUX_INFO("TcpAcceptor Start OK, local ip:{0} port:{1}", local_ip_, local_port_);
    return true;
}


bool TcpAcceptor::Stop() {
    Close();
    MUX_INFO("TcpAcceptor Stop OK");
}

void TcpAcceptor::Close() {
    ::close(handle_);
    handle_ = -1;
    MUX_INFO("close tcp acceptor");
}

int32_t TcpAcceptor::CreateSocket() {
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

int32_t TcpAcceptor::MakeSocketNonBlock(int32_t fd) {
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

int32_t TcpAcceptor::Listen(int32_t listenfd) {
    int r = ::listen(listenfd, SOMAXCONN);
    if ( r < 0) {
        MUX_ERROR("listen failed");
        return -1;
    }
    return 0;
}

void TcpAcceptor::OnSocketAccept() {
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
        MUX_DEBUG("accept connection from {0}", inet_ntoa(in_addr.sin_addr));
        fprintf(stdout, "accept connection from %s:%u\n", inet_ntoa(in_addr.sin_addr), in_addr.sin_port);
        fflush(stdout);
        int mr = MakeSocketNonBlock(cli_fd);
        if (mr < 0) {
            ::close(cli_fd);
            continue;
        }

        int er = UpdateEpollEvents(efd_, EPOLL_CTL_ADD, cli_fd, EPOLLIN | EPOLLOUT| EPOLLRDHUP | EPOLLET);
        if (er < 0 ) {
            ::close(cli_fd);
            continue;
        }
    }
}


} // end namespace transport
} // end namespace mux
