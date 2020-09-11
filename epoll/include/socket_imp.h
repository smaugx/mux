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


using callback_recv_t    = std::function<void(transport::PacketPtr&)>;
using callback_accept_t  = std::function<void(uint32_t eid)>;

class SocketBase {
public:
    SocketBase(const SocketBase&)              = delete;
    SocketBase& operator=(const SocketBase&)   = delete;
    SocketBase(SocketBase&&)                   = delete;
    SocketBase& operator=(SocketBase&&)        = delete;

    SocketBase()                               = default;
    virtual ~SocketBase()                      = default;

public:
    virtual void HandleRead()                           = 0;
    virtual void HandleWrite()                          = 0;
    virtual void HandleError()                          = 0;
    virtual int32_t SendData(const std::string& data)   = 0;
    virtual int32_t SendData(const PacketPtr& packet)   = 0;
    virtual void Close()                                = 0;
    virtual std::string GetLocalIp()                    = 0;
    virtual uint16_t GetLocalPort()                     = 0;
    virtual std::string GetRemoteIp()                   = 0;
    virtual uint16_t GetRemotePort()                    = 0;
    virtual int32_t GetDescriptor()                     = 0;

};


class BasicSocket : public SocketBase {
public:
    BasicSocket(const BasicSocket&)              = delete;
    BasicSocket& operator=(const BasicSocket&)   = delete;
    BasicSocket(BasicSocket&&)                   = delete;
    BasicSocket& operator=(BasicSocket&&)        = delete;

public:
    BasicSocket();
    BasicSocket(int fd); // for server listen
    BasicSocket(
            int fd,
            const std::string& local_ip,
            uint16_t local_port,
            const std::string& remote_ip,
            uint16_t remote_port);
    virtual ~BasicSocket();

public:
    void HandleRead() override;
    void HandleWrite() override;
    void HandleError() override;
    int32_t SendData(const std::string& data) override;
    int32_t SendData(const PacketPtr& packet) override;
    void Close() override;

    inline std::string GetLocalIp() override {
        return local_ip_;
    }
    
    inline uint16_t GetLocalPort() override {
        return local_port_;
    }

    inline std::string GetRemoteIp() override {
        return remote_ip_;
    }

    inline uint16_t GetRemotePort() override {
        return remote_port_;
    }

    inline int32_t GetDescriptor() override {
        return fd_;
    }

protected:
    // handle recv data, rewrite this function of yourself
    virtual int32_t HandleRecvData(const PacketPtr& packet);

private:
    int fd_ { -1 };
    std::string local_ip_;
    uint16_t local_port_ { 0 };
    std::string remote_ip_;
    uint16_t remote_port_ { 0 };
    bool closed_ { true };

};

} // end namespace transport

} // end namespace mux



