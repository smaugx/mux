#include "epoll_tcp_socket.h"


namespace mux {

namespace EpollTcpSocket {

static const uint32_t kEpollWaitTime = 10; // 10 ms
static const uint32_t kMaxEvents = 100;

EpollTcpSocket::EpollTcpSocket(const std::string& local_ip, uint16_t local_port)
    : local_ip_ { local_ip },
      local_port_ { local_port } {
}

EpollTcpSocket::~EpollTcpSocket() {
}

bool EpollTcpSocket::Init() {
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
    std::cout << "EpollTcpSocket Init success!" << std::endl;
    handle_ = listenfd;

    assert(!th_loop_);

    th_loop_ = std::make_shared<std::thread>(std::bind(&EpollTcpSocket::Start, this));
    if (!th_loop_) {
        return false;
    }

    return true;
}

int32_t EpollTcpSocket::CreateEpoll() {
    int epollfd = epoll_create(1);
    if (epollfd < 0) {
        std::cout << "epoll_create failed!" << std::endl;
        return -1;
    }
    efd_ = epollfd;
    return epollfd;
}

int32_t EpollTcpSocket::CreateAndBind() {
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

int32_t EpollTcpSocket::MakeSocketNonBlock(int32_t fd) {
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

int32_t EpollTcpSocket::Listen(int32_t listenfd) {
    int r = ::listen(listenfd, SOMAXCONN);
    if ( r < 0) {
        std::cout << "listen failed!" << std::endl;
        return -1;
    }
    return 0;
}

int32_t EpollTcpSocket::UpdateEpollEvents(int efd, int fd, int events, int op) {
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

void EpollTcpSocket::Start() {
    struct epoll_event* alive_events =  static_cast<epoll_event*>(malloc(sizeof(epoll_event) * kMaxEvents));
    while (loop_flag_) {
        int num = epoll_wait(efd_, alive_events, kMaxEvents, kEpollWaitTime);
        for (int i = 0; i < num; ++i) {
            int fd = alive_events[i].data.fd;
            int events = alive_events[i].events;
            // TODO(smaug)
        }
    }
}

} // end namespace transport
} // end namespace mux
