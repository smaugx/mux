//#include "epoll/include/socket_imp.h"
#include "socket_imp.h"

#include <memory>


#include "mbase/mux_log.h"

namespace mux {

namespace transport {

Socket::Socket() {
    sock_core_ = new SocketCore();
    sock_core_->register_event_handler(std::bind(&Socket::SocketHandler, this, std::placeholders::_1));
}

Socket::Socket(int fd) {
    sock_core_ = new SocketCore(fd);
    // do not register event_handler callback
}

Socket::Socket(
        int fd,
        const std::string& local_ip,
        uint16_t local_port,
        const std::string& remote_ip,
        uint16_t remote_port) {
    sock_core_ = new SocketCore(fd, local_ip, local_port, remote_ip, remote_port);
    sock_core_->register_event_handler(std::bind(&Socket::SocketHandler, this, std::placeholders::_1));
}

void* Socket::GetSocketImp() {
    return (void*)(sock_core_);
}

void Socket::Close() {
    if (!sock_core_ || sock_core_->closed_) {
        return;
    }
    MUX_INFO("Socket::Close {0}", sock_core_->fd_);
    ::close(sock_core_->fd_);
    sock_core_->closed_ = true; // no need
    delete sock_core_;
    sock_core_ = nullptr;
}

int32_t Socket::SendData(const std::string& data) {
    PacketPtr packet = std::make_shared<Packet>(data);
    packet->from_ip_addr_ = sock_core_->local_ip_;
    packet->from_ip_port_ = sock_core_->local_port_;
    packet->to_ip_addr_   = sock_core_->remote_ip_;
    packet->to_ip_port_   = sock_core_->remote_port_;
    return SendData(packet);
}

int32_t Socket::SendData(const PacketPtr& packet) {
    if (!sock_core_ || sock_core_->closed_) {
        MUX_ERROR("send failed, socket closed");
        return -1;
    }
    int r = ::write(sock_core_->fd_, packet->msg_.data(), packet->msg_.size());
    if (r == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            MUX_INFO("send {0} bytes finished", packet->msg_.size());
            return -1;
        }
        // error happend
        Close();
        MUX_ERROR("write error, will close this fd:{0}", sock_core_->fd_);
        return -1;
    }
    MUX_DEBUG("write size:{0} in fd:{1} ok", r, sock_core_->fd_);
    return r;
}

void Socket::SocketHandler(int32_t event_type) {
    if (event_type == ERR_EVENT) {
        MUX_ERROR("Socket: fd={0} remote_endpoint={1}:{2} event_type: error", sock_core_->fd_, sock_core_->remote_ip_, sock_core_->remote_port_);
        Close();
    } else if (event_type == READ_EVENT) {
        MUX_DEBUG("Socket: fd={0} remote_endpoint={1}:{2} event_type: read", sock_core_->fd_, sock_core_->remote_ip_, sock_core_->remote_port_);
        char read_buf[4096];
        bzero(read_buf, sizeof(read_buf));
        int n = -1;
        while ( (n = ::read(sock_core_->fd_, read_buf, sizeof(read_buf))) > 0) {
            // callback for recv
            std::string msg(read_buf, n);
            PacketPtr packet = std::make_shared<Packet>(msg);
            MUX_DEBUG("recv_size:{0} from fd:{1} remote:{2}:{3}", n, sock_core_->fd_, packet->from_ip_addr_, packet->from_ip_port_);
            if (recv_callback_) {
                recv_callback_(packet);
            } else {
                MUX_WARN("no recv callback reigistered!");
            }
        }
        if (n == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // read finished
                MUX_DEBUG("read finished in fd:{0}", sock_core_->fd_);
                return;
            }
            // something goes wrong for this fd, should close it
            Close();
            MUX_ERROR("something goes wrong, will close this fd:{0}.", sock_core_->fd_);
            return;
        }
        if (n == 0) {
            // this may happen when client close socket. EPOLLRDHUP usually handle this, but just make sure; should close this fd
            Close();
            MUX_ERROR("peer maybe closed, will close this fd:{0}.", sock_core_->fd_);
            return;
        }
    } else if (event_type == WRITE_EVENT) {
        MUX_DEBUG("Socket: fd={0} remote_endpoint={1}:{2} event_type: write", sock_core_->fd_, sock_core_->remote_ip_, sock_core_->remote_port_);
        // TODO(smaug)
    } else {
        MUX_ERROR("invalid event_type:{0}", event_type);
    }
    return;
}



} // end namespace transport

} // end namespace mux
