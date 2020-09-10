#pragma once

#include <unistd.h>

#include <string>
#include <memory>
#include <functional>

#include "mbase/packet.h"

namespace mux {

namespace transport {

static const int32_t ERR_EVENT   = -1;
static const int32_t READ_EVENT  = 0;
static const int32_t WRITE_EVENT = 1;

using SocketHandlerFunc = std::function<void(int32_t)>;

class SocketCore {
public:
    SocketCore()                               = default;
    SocketCore(const SocketCore&)              = delete;
    SocketCore& operator=(const SocketCore&)   = delete;
    SocketCore(SocketCore&&)                   = delete;
    SocketCore& operator=(SocketCore&&)        = delete;

public:
    SocketCore(int fd)
        : fd_(fd),
          closed_(false) {}

    SocketCore(
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
          closed_(false) {}


    ~SocketCore() {
        if (!closed_) {
            ::close(fd_);
            closed_ = true;
        }
    }

public:
    inline void register_event_handler(SocketHandlerFunc call) {
        if (sock_call_) {
            return;
        }
        sock_call_ = call;
    }

    inline void notify_socket_event(int32_t event_type) {
        if (!sock_call_) {
            return;
        }
        sock_call_(event_type);
    }

public:
    int fd_ { -1 };
    std::string local_ip_;
    uint16_t local_port_ { 0 };
    std::string remote_ip_;
    uint16_t remote_port_ { 0 };
    SocketHandlerFunc sock_call_;
    bool closed_ { true };
};


class Socket {
public:
    Socket(const Socket&)              = delete;
    Socket& operator=(const Socket&)   = delete;
    Socket(Socket&&)                   = delete;
    Socket& operator=(Socket&&)        = delete;

public:
    Socket();
    explicit Socket(int fd);  // special for listen handle
    Socket(
            int fd,
            const std::string& local_ip,
            uint16_t local_port,
            const std::string& remote_ip,
            uint16_t remote_port);

public:
    int32_t SendData(const std::string& data);
    int32_t SendData(const PacketPtr& packet);
    void Close();
    void RegisterOnRecvCallback(callback_recv_t callback);
    void UnRegisterOnRecvCallback();
    void* GetSocketImp();

protected:
    void SocketHandler(int32_t event_type);

private:
    SocketCore* sock_core_ { nullptr };
    callback_recv_t recv_callback_ { nullptr };
};

typedef std::shared_ptr<Socket> SocketPtr;


} // end namespace transport

} // end namespace mux



