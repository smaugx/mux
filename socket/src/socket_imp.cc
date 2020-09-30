#include "socket/include/socket_imp.h"
#include "mbase/include/mux_log.h"
#include "mbase/include/mux_utils.h"

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
    char header_buf[sizeof(packet_header)];
    bzero(header_buf, sizeof(header_buf));
    int n = -1;
    while (true) {
        n = ::read(fd_, header_buf, sizeof(packet_header));
        if (n <= 0) {
            // read data finished or error
            break;
        }
        if (n != sizeof(packet_header)) {
            // read header size invalid
            MUX_WARN("read packet header size invalid:{0}", n);
            continue;
        } 

        mux::PacketPtr packet = std::make_shared<mux::Packet>();
        memcpy(&(packet->header()), header_buf, sizeof(packet_header));
        // check packet_len is legal
        if (packet->header().packet_len <= 0 || packet->header().packet_len > PACKET_LEN_MAX) {
            MUX_WARN("packet header invalid, read packet len:{0}",  packet->header().packet_len);
            continue;
        }


        // begin read packet body using packet_len
        MUX_DEBUG("read packet header: packet_len:{0} binary_protocol:{1}", packet->header().packet_len, packet->header().binary_protocol);
        uint16_t packet_read = 0;  // how many bytes have been readed
        int pn = -1;
        char read_buf[4096];
        bzero(read_buf, sizeof(read_buf));
        while (packet_read < packet->header().packet_len) {
            int packet_read_left = packet->header().packet_len - packet_read; // how many bytes left to read
            int read_size  = (packet_read_left > 4096)?4096:packet_read_left;
            pn = ::read(fd_, read_buf, read_size);
            if (pn == 0) {
                // this may happen when client close socket. EPOLLRDHUP usually handle this, but just make sure; should close this fd
                Close();
                MUX_ERROR("peer maybe closed, will close this fd:{0}.", fd_);
                return;
            }

            if (pn == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // read finished, but body not finished ,continue
                    MUX_DEBUG("read packet body finished, but body not whole, continue wait to read body");
                    continue;
                }
                Close();
                MUX_ERROR("peer maybe closed, will close this fd:{0}.", fd_);
                return;
            }

            packet->body() += std::string(read_buf, pn);
            packet_read += pn;
            bzero(read_buf, sizeof(read_buf));
        }

        // illegal packet
        if (packet_read != packet->header().packet_len) {
            // usually not happend
            MUX_WARN("read packet body invalid packet_read:{0} packet_len:{1}", packet_read, packet->header().packet_len);
            continue;
        }

        // legal packet
        MUX_DEBUG("read packet body size:{0}", packet_read);
        packet->set_from_ip_addr(remote_ip_);
        packet->set_from_ip_port(remote_port_);

        // using callback or using virtual function both is ok
        if (callback_) {
            callback_(packet);
        }

    } // end while(true)

    if (n == 0) {
        // this may happen when client close socket. EPOLLRDHUP usually handle this, but just make sure; should close this fd
        Close();
        MUX_ERROR("peer maybe closed, will close this fd:{0}.", fd_);
        return;
    }

    if (n == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // read finished
            MUX_DEBUG("read packet finished in fd:{0}", fd_);
            return;
        }
        // something goes wrong for this fd, should close it
        Close();
        MUX_ERROR("something goes wrong, will close this fd:{0}.", fd_);
        return;
    }
    */

    int n = -1;
    MUX_DEBUG("in_buf capacity:{0} size:{1} free_size:{2}", in_buf_.capacity(), in_buf_.size(), in_buf_.free_size());
    while ( (n = ::read(fd_, in_buf_.write_head(), in_buf_.free_size()) > 0)) {
        MUX_DEBUG("read size {0} from remote {1}:{2}", n, remote_ip_, remote_port_);
        in_buf_.commit(n);
        MUX_DEBUG("in_buf capacity:{0} size:{1} free_size:{2}", in_buf_.capacity(), in_buf_.size(), in_buf_.free_size());

        // try read packet
        if (in_buf_.size() < PACKET_HEAD_SIZE) {
            continue;
        }
        // size beyond packet head size
        while (in_buf_.size() >= PACKET_HEAD_SIZE) {
            // just read head, no need consume
            packet_header header;
            memcpy(&header, in_buf_.read_head(), PACKET_HEAD_SIZE);
            // check packet_len is legal
            if (header.packet_len <= 0 || header.packet_len > PACKET_LEN_MAX) {
                MUX_WARN("packet header invalid, read packet len:{0}, close this fd:{1}",  header.packet_len, fd_);
                Close();
                return;
            }
            if (in_buf_.size() >= (PACKET_HEAD_SIZE + header.packet_len)) {
                // at list one whole packet
                mux::PacketPtr packet = std::make_shared<mux::Packet>(in_buf_.read_head(), PACKET_HEAD_SIZE + header.packet_len);
                in_buf_.consume(PACKET_HEAD_SIZE + header.packet_len);
                MUX_DEBUG("in_buf capacity:{0} size:{1} free_size:{2}", in_buf_.capacity(), in_buf_.size(), in_buf_.free_size());

                // using callback or using virtual function both is ok
                if (callback_) {
                    MUX_DEBUG("callback for packet");
                    callback_(packet);
                }
            } else {
                // not enough bytes for a whole packet
                break;
            }
        } // end while (in_buf_.size() >= PACKET_HEAD_SIZE)
    } // end while((n=::read(...

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




    /*
    char read_buf[4096];
    bzero(read_buf, sizeof(read_buf));
    int n = -1;
    while ( (n = ::read(fd_, read_buf, sizeof(read_buf))) > 0) {
        // callback for recv
        MUX_DEBUG("recv_size:{0} from fd:{1}", n, fd_);
        std::string msg(read_buf, n);
        PacketPtr packet = std::make_shared<Packet>(msg);
        packet->set_from_ip_addr(remote_ip_);
        packet->set_from_ip_port(remote_port_);
        packet->set_to_ip_addr(local_ip_);
        packet->set_to_ip_port(local_port_);
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
    */

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
    if (packet->body_size() <= 0) {
        MUX_WARN("packet invalid body_size:{1}", packet->body_size());
        return -1;
    }

    // send header + body
    int r = ::write(fd_, packet->data(), packet->size());
    if (r == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            MUX_INFO("send {0} bytes finished", packet->size());
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
