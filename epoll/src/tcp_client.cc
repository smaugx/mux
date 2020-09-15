#include "epoll/include/tcp_client.h"

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


namespace mux {

namespace transport {

static const uint32_t kEpollWaitTime = 10; // 10 ms
static const uint32_t kMaxEvents = 100;

TcpClient::TcpClient(const std::string& server_ip, uint16_t server_port)
    : MuxSocket(server_ip, server_port) {}

TcpClient::~TcpClient() {
    Stop();
}

bool TcpClient::Start() {
    // create socket and bind
    int cli_fd  = CreateSocket();
    if (cli_fd < 0) {
        return false;
    }

    fd_ = cli_fd;
    int lr = Connect(fd_);
    if (lr < 0) {
        return false;
    }
    lr = MakeSocketNonBlock(fd_);
    if (lr < 0) {
        return false;
    }
    closed_ = false;
    MUX_INFO("TcpClient Start OK, target ip:{0} port:{1} fd:{2}", remote_ip_, remote_port_, fd_);

    return true;
}

int32_t TcpClient::CreateSocket() {
    int cli_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (cli_fd < 0) {
        MUX_ERROR("create socket failed!");
        return -1;
    }

    return cli_fd;
}

int32_t TcpClient::Connect(int32_t cli_fd) {
    struct sockaddr_in addr;  // server info
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(remote_port_);
    addr.sin_addr.s_addr  = inet_addr(remote_ip_.c_str());

    int r = ::connect(cli_fd, (struct sockaddr*)&addr, sizeof(addr));
    if ( r < 0) {
        MUX_ERROR("connect {0} {1} failed!", remote_ip_, remote_ip_);
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

int32_t TcpClient::MakeSocketNonBlock(int32_t fd) {
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

/*
int32_t TcpClient::HandleRecvData(const PacketPtr& packet) {
    MUX_DEBUG("in TcpClient::HandleRecvData");
    return 0;
}
*/


} // end namespace transport
} // end namespace mux
