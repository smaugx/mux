#include "epoll/include/socket_imp.h"

namespace mux {

namespace transport {

BasicSocket::BasicSocket()
    : closed_(true) {}

BasicSocket::BasicSocket(int fd)
    : fd_(fd),
      closed_(true) {}

BasicSocket::BasicSocket(
        int fd,
        const std::string& local_ip,
        uint16_t local_port,
        const std::string& remote_ip,
        uint16_t remote_port)
    : fd_(fd),
      local_ip_(local_ip),
      local_port_(local_port),
      remote_ip_(remote_ip),
      remote_port_(remote_port),
      closed_(false) {
    MUX_INFO("create socket(0) local {1}:{2} remote {3}:{4}", fd_, local_ip_, local_port_, remote_ip_, remote_port_);
}

void BasicSocket::handle_read() {
    char read_buf[4096];
    bzero(read_buf, sizeof(read_buf));
    int n = -1;
    while ( (n = ::read(fd_, read_buf, sizeof(read_buf))) > 0) {
        // callback for recv
        MUX_DEBUG("recv_size:{0} from fd:{1}", n, fd_);
        std::string msg(read_buf, n);
        PacketPtr packet = std::make_shared<Packet>(msg);
        // finally handle recv data
        HandleRecvData(packet);
    }
    if (n == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // read finished
            MUX_DEBUG("read finished in fd:{0}", fd_);
            return;
        }
        // something goes wrong for this fd, should close it
        Close();
        MUX_ERROR("something goes wrong, will close this fd:{0}.", fd_);
        return;
    }
    if (n == 0) {
        // this may happen when client close socket. EPOLLRDHUP usually handle this, but just make sure; should close this fd
        Close();
        MUX_ERROR("peer maybe closed, will close this fd:{0}.", fd_);
        return;
    }
    return;
}

void BasicSocket::handle_write() {
    // TODO(smaug)
    MUX_INFO("handle write(TODO)");
}

void BasicSocket::handle_error() {
    Close();
}

int32_t BasicSocket::SendData(const std::string& data) {
    if (closed_) {
        MUX_ERROR("socket closed, not ready for send");
        return -1;
    }
    auto packet = std::make_shared<Packet>(data);
    return SendData(packet);
}

int32_t BasicSocket::SendData(const PacketPtr& packet) {
    if (closed_) {
        MUX_ERROR("socket closed, not ready for send");
        return -1;
    }
    int r = ::write(fd_, packet->msg_.data(), packet->msg_.size());
    if (r == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            MUX_INFO("send {0} bytes finished", packet->msg_.size());
            return -1;
        }
        // error happend
        Close();
        MUX_ERROR("write error, will close this fd:{0}", fd_);
        return -1;
    }
    MUX_DEBUG("write size:{0} in fd:{1} ok", r, fd_);
    return r;
}

void BasicSocket::Close() {
    MUX_INFO("close socket(0) local {1}:{2} remote {3}:{4}", fd_, local_ip_, local_port_, remote_ip_, remote_port_);
    ::close(fd_);
    fd_ = -1;
    closed_ = true;
}

int32_t BasicSocket::HandleRecvData(const PacketPtr& packet) {
    // handle recv here
    MUX_DEBUG("handle recv data");
    MUX_INFO("recv packet size:{0} content:{1}", (packet->msg_).size(), packet->msg_);
}


} // end namespace transport

} // end namespace mux
