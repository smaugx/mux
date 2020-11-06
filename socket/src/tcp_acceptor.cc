#include "socket/include/tcp_acceptor.h"

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

#include "mbase/include/mux_log.h"
#include "socket/include/socket_imp.h"

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
    ClearConnections();
    MUX_INFO("TcpAcceptor Stop OK");
    return true;
}

void TcpAcceptor::Close() {
    if (handle_ == -1) {
        return;
    }
    ::close(handle_);
    handle_ = -1;
    MUX_INFO("close tcp acceptor");
}

void TcpAcceptor::ClearConnections() {
    std::unique_lock<std::mutex> lock(connection_vec_mutex_);
    for (const auto& conn : connection_vec_) {
        MUX_INFO("close and remove connection {0}:{1}", conn->GetRemoteIp(), conn->GetRemotePort());
        delete conn;
    }
}

int32_t TcpAcceptor::CreateSocket() {
    int listenfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        MUX_ERROR("create socket failed!");
        return -1;
    }

    /*
    // add reuseport
    int ret = 0;
    int reuse = 1;
    ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,(const void *)&reuse , sizeof(int));
    if (ret < 0) {
        perror("setsockopt");
	return -1;
    }
    ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT,(const void *)&reuse , sizeof(int));
    if (ret < 0) {
        perror("setsockopt");
	return -1;
    }
    */

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

void TcpAcceptor::RegisterNewSocketRecvCallback(callback_recv_t callback) {
    if (new_socket_recv_callback_) {
        MUX_WARN("new_socket_recv_callback_ already registered");
        return;
    }
    new_socket_recv_callback_ = callback;
}

BasicSocket* TcpAcceptor::ManageNewConnection(BasicSocket* new_sock) {
    if (new_socket_recv_callback_) {
        new_sock->RegisterOnRecvCallback(new_socket_recv_callback_);
    } else {
        MUX_WARN("new_socket_recv_callback_ not ready, handle recv failed");
    }

    {
        std::unique_lock<std::mutex> lock(connection_vec_mutex_);
        connection_vec_.push_back(new_sock);
        MUX_INFO("add new connection {0}:{1}", new_sock->GetRemoteIp(), new_sock->GetRemotePort());
    }

    return new_sock;
}

BasicSocket* TcpAcceptor::OnSocketAccept(int32_t cli_fd, const std::string& remote_ip, uint16_t remote_port) {
    BasicSocket* new_sock = new MuxSocket(cli_fd, local_ip_, local_port_, remote_ip, remote_port);
    if (!new_sock) {
        MUX_ERROR("error create muxsocket");
        return nullptr;
    }

    // handle your session here
    

    return ManageNewConnection(new_sock);
}


} // end namespace transport
} // end namespace mux
