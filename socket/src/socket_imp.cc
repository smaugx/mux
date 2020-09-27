#include "socket/include/socket_imp.h"
#include "mbase/include/mux_log.h"

namespace mux {

namespace transport {

MuxSocket::MuxSocket(
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
    MUX_INFO("create socket{0} local {1}:{2} remote {3}:{4}", fd_, local_ip_, local_port_, remote_ip_, remote_port_);
}

MuxSocket::MuxSocket(
        const std::string& remote_ip,
        uint16_t remote_port)
    : remote_ip_(remote_ip),
      remote_port_(remote_port),
      closed_(true) {
    MUX_INFO("client create socket{0} local {1}:{2} remote {3}:{4}", fd_, local_ip_, local_port_, remote_ip_, remote_port_);
}

MuxSocket::~MuxSocket() {
    Close();
}

void MuxSocket::HandleRead() {
    /*
    char header_buf[PACKET_HEADER_SIZE + 1];
    bzero(header_buf, sizeof(header_buf));
    int n = -1;
    while (true) {
        n = ::read(fd_, header_buf, sizeof(header_buf));
        if (n <= 0) {
            // read data finished or error
            break;
        }

        uint16_t packet_len;
        memcpy(&packet_len, header_buf, sizeof(packet_len));
        // check packet_len is legal
        if (packet_len <= 0 || packet_len > PACKET_LEN_MAX) {
            MUX_WARN("packet header invalid, read packet len:{0}",  packet_len);
            continue;
        }

        // begin read packet body using packet_len
        uint16_t packet_read = 0;
        int pn = -1;
        char read_buf[PACKET_LEN_MAX + 1];
        bzero(read_buf, sizeof(read_buf));
        while (packet_read < packet_len) {
            pn = ::read(fd_, read_buf, sizeof(read_buf));
        }

    }
    */



    char read_buf[4096];
    bzero(read_buf, sizeof(read_buf));
    int n = -1;
    while ( (n = ::read(fd_, read_buf, sizeof(read_buf))) > 0) {
        // callback for recv
        MUX_DEBUG("recv_size:{0} from fd:{1}", n, fd_);
        std::string msg(read_buf, n);
        PacketPtr packet = std::make_shared<Packet>(msg);
        packet->from_ip_addr = remote_ip_;
        packet->from_ip_port = remote_port_;
        // finally handle recv data
        //HandleRecvData(packet);

        // using callback or using virtual function both is ok
        if (callback_) {
            callback_(packet);
        }

        bzero(read_buf, sizeof(read_buf));
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

void MuxSocket::HandleWrite() {
    // TODO(smaug)
    MUX_INFO("handle write(TODO)");
}

void MuxSocket::HandleError() {
    Close();
}

int32_t MuxSocket::SendData(const std::string& data) {
    if (closed_) {
        MUX_ERROR("socket closed, not ready for send");
        return -1;
    }
    auto packet = std::make_shared<Packet>(data);
    return SendData(packet);
}

int32_t MuxSocket::SendData(const PacketPtr& packet) {
    if (closed_) {
        MUX_ERROR("socket closed, not ready for send");
        return -1;
    }

    int hr = ::write(fd_, &(packet->header), sizeof(packet->header));
    if (hr == -1) {
        MUX_WARN("send packet header failed");
        return -1;
    }

    int r = ::write(fd_, packet->msg.data(), packet->msg.size());
    if (r == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            MUX_INFO("send {0} bytes finished", packet->msg.size());
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

void MuxSocket::Close() {
    if (closed_) {
        return;
    }
    MUX_INFO("close socket(0) local {1}:{2} remote {3}:{4}", fd_, local_ip_, local_port_, remote_ip_, remote_port_);
    ::close(fd_);
    fd_ = -1;
    closed_ = true;
}

int32_t MuxSocket::HandleRecvData(const PacketPtr& packet) {
    // handle recv here
    MUX_DEBUG("handle recv data");
    MUX_INFO("recv packet size:{0} content:{1}", (packet->msg).size(), packet->msg);
    return 0;
}

void MuxSocket::RegisterOnRecvCallback(callback_recv_t callback) {
    if (callback_) {
        return;
    }
    callback_ = callback;
    MUX_INFO("register recv callback for socket {0}:{1}", remote_ip_, remote_port_);
}


} // end namespace transport

} // end namespace mux
