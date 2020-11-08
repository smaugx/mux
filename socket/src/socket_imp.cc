#include <algorithm>

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
    MUX_INFO("client create socket:{0} local {1}:{2} remote {3}:{4}", fd_, local_ip_, local_port_, remote_ip_, remote_port_);
}

MuxSocket::~MuxSocket() {
    Close();
}

void MuxSocket::HandleRead() {
    int n = -1;
    MUX_DEBUG("in_buf capacity:{0} size:{1} free_size:{2}", in_buf_.capacity(), in_buf_.size(), in_buf_.free_size());
    char read_buf[4096];
    bzero(read_buf, sizeof(read_buf));
    uint32_t read_max_size = std::min((size_t)in_buf_.free_size(), (size_t)4096);
    while ( (n = ::read(fd_, read_buf, read_max_size)) > 0) {
        MUX_DEBUG("read size {0} from remote {1}:{2}", n, remote_ip_, remote_port_);
        in_buf_.write(read_buf, n);
        bzero(read_buf, sizeof(read_buf));
        MUX_DEBUG("in_buf capacity:{0} size:{1} free_size:{2}", in_buf_.capacity(), in_buf_.size(), in_buf_.free_size());

        // try read packet
        if (in_buf_.size() < PACKET_HEAD_SIZE) {
            read_max_size = std::min((size_t)in_buf_.free_size(), (size_t)4096);
            continue;
        }
        // size beyond packet head size
        while (in_buf_.size() >= PACKET_HEAD_SIZE) {
            // just read head, no need consume
            packet_header header;
            in_buf_.peak(&header, PACKET_HEAD_SIZE);

            /*
            std::cout << "read header packet_len:" << (uint32_t)header.packet_len << std::endl;
            std::cout << "read header binary_protocol:" << (uint32_t)header.binary_protocol << std::endl;
            std::cout << "read header priority:" << (uint32_t)header.priority << std::endl;
            */

            // check packet_len is legal
            if (header.packet_len <= 0 || header.packet_len > PACKET_LEN_MAX) {
                MUX_WARN("packet header invalid, read packet len:{0}, close this fd:{1}",  header.packet_len, fd_);
                Close();
                return;
            }
            MUX_DEBUG("packet header ok, len:{0}", header.packet_len);
            if (in_buf_.size() >= (PACKET_HEAD_SIZE + header.packet_len)) {
                MUX_DEBUG("in_buf capacity:{0} size:{1} free_size:{2}", in_buf_.capacity(), in_buf_.size(), in_buf_.free_size());
                // at list one whole packet
                mux::PacketPtr packet = std::make_shared<mux::Packet>(PACKET_HEAD_SIZE + header.packet_len);
                in_buf_.read((void*)packet->data(), PACKET_HEAD_SIZE + header.packet_len);

                // using callback or using virtual function both is ok
                if (callback_) {
                    packet->set_to_ip_addr(local_ip_);
                    packet->set_to_ip_port(local_port_);
                    packet->set_from_ip_addr(remote_ip_);
                    packet->set_from_ip_port(remote_port_);
                    MUX_DEBUG("callback for packet");
                    callback_(packet);
                }
            } else {
                // not enough bytes for a whole packet
                break;
            }
        } // end while (in_buf_.size() >= PACKET_HEAD_SIZE)

        read_max_size = std::min((size_t)in_buf_.free_size(), (size_t)4096);
    } // end while((n=::read(...

    if (n == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // read finished
            MUX_DEBUG("read finished in fd:{0}", fd_);
            return;
        }
        // something goes wrong for this fd, should close it
        Close();
        MUX_ERROR("something goes wrong, will close this fd:{0} errno:{1}", fd_, errno);
        return;
    }
    if (n == 0) {
        // this may happen when client close socket. EPOLLRDHUP usually handle this, but just make sure; should close this fd
        Close();
        MUX_ERROR("peer maybe closed, will close this fd:{0} errno:{1}", fd_, errno);
        return;
    }

    return;
}

void MuxSocket::HandleWrite() {
    // TODO(smaug)
    MUX_INFO("handle write(TODO)");
}

void MuxSocket::HandleError() {
    MUX_WARN("handle error, fd:{0}", fd_);
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

    //return SendBinary(packet->data(), packet->size());

    int r = ::write(fd_, packet->data(), packet->size());
    if (r == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            MUX_INFO("send {0} bytes finished", packet->size());
            return -1;
        }
        // error happend
        Close();
        MUX_ERROR("write error, will close this fd:{0} errno:{1}", fd_, errno);
        return -1;
    }
    MUX_DEBUG("write size:{0} in fd:{1} ok", r, fd_);
    return r;
}

int32_t MuxSocket::SendBinary(const uint8_t *data, uint32_t size) {
    if (closed_) {
        MUX_ERROR("socket closed, not ready for send");
        return -1;
    }
    if (size <= 0 || size > RINGBUF_OUT_SIZE) {
        MUX_WARN("packet invalid body_size:{1}", size);
        return -1;
    }
    if (out_buf_.size() > 0) {
        MUX_INFO("send failed, send cache not empty, remain {0} bytes", out_buf_.size());
        return -1;
    }

    int r = -1;
    uint32_t send_bytes = 0;
    while (true) {
        // send header + body
        r = ::write(fd_, data + send_bytes, size - send_bytes);
        if (r > 0) {
            if (r == (size - send_bytes)) {
                MUX_DEBUG("write size:{0} in fd:{1} ok", r, fd_);
                return r;
            } else {
                // wait next while loop 
                send_bytes += r;
                MUX_DEBUG("write part size:{0}, remain {1} bytes", send_bytes, size - send_bytes);
            }
        } else if (r == 0){
            // something goes wrong maybe
            Close();
            MUX_ERROR("write error, will close this fd:{0} errno:{1}", fd_, errno);
            return -1;
        } else { // r == -1
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // write to out_ringbuffer for next epollout, then send again 
                out_buf_.write(data + send_bytes, size - send_bytes);
                MUX_INFO("send {0} bytes finished, remain {1} bytes to send next time, write to cache", r, size - r);
                return -1;
            }
            // something goes wrong maybe
            Close();
            MUX_ERROR("write error, will close this fd:{0} errno:{1}", fd_, errno);
            return -1;
        }
    } // end while (true)...

    // should not come here
    return -1;
}

void MuxSocket::Close() {
    if (closed_) {
        return;
    }
    MUX_INFO("close socket:{0} local {1}:{2} remote {3}:{4}", fd_, local_ip_, local_port_, remote_ip_, remote_port_);
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
