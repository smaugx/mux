#pragma once

#include <unistd.h>

#include <string>
#include <memory>
#include <functional>

#include "mbase/include/packet.h"
#include "mbase/include/runnable.h"
#include "socket/include/ringbuffer.h"

namespace mux {

namespace transport {

static const int32_t ERR_EVENT   = -1;
static const int32_t READ_EVENT  = 0;
static const int32_t WRITE_EVENT = 1;
static const uint32_t RINGBUF_IN_SIZE = 20 * 1024; // 20KB
static const uint32_t RINGBUF_OUT_SIZE = 20 * 1024; // 20KB

using RingBuffer =  transport::ring_buffer_s;


class SocketBase : public RunEntity {
public:
    SocketBase(const SocketBase&)              = delete;
    SocketBase& operator=(const SocketBase&)   = delete;
    SocketBase(SocketBase&&)                   = delete;
    SocketBase& operator=(SocketBase&&)        = delete;

    SocketBase()                               = default;
    virtual ~SocketBase()                      = default;
public:
    virtual std::string GetLocalIp()                    = 0;
    virtual uint16_t GetLocalPort()                     = 0;
    virtual int32_t GetDescriptor()                     = 0;
    virtual bool CheckListener()                        = 0;
    virtual void Close()                                = 0;
};


class BasicSocket : public SocketBase {
public:
    BasicSocket(const BasicSocket&)              = delete;
    BasicSocket& operator=(const BasicSocket&)   = delete;
    BasicSocket(BasicSocket&&)                   = delete;
    BasicSocket& operator=(BasicSocket&&)        = delete;

    BasicSocket()                                = default;
    virtual ~BasicSocket()                       = default;

public:
    virtual void HandleRead()                                     = 0;
    virtual void HandleWrite()                                    = 0;
    virtual void HandleError()                                    = 0;
    virtual int32_t SendData(const std::string& data)             = 0;
    virtual int32_t SendData(const PacketPtr& packet)             = 0;
    virtual std::string GetRemoteIp()                             = 0;
    virtual uint16_t GetRemotePort()                              = 0;
    virtual void RegisterOnRecvCallback(callback_recv_t callback) = 0;
};


class MuxSocket : public BasicSocket {
public:
    MuxSocket(const MuxSocket&)              = delete;
    MuxSocket& operator=(const MuxSocket&)   = delete;
    MuxSocket(MuxSocket&&)                   = delete;
    MuxSocket& operator=(MuxSocket&&)        = delete;

public:
    MuxSocket(
            int fd,
            const std::string& local_ip,
            uint16_t local_port,
            const std::string& remote_ip,
            uint16_t remote_port);
    MuxSocket(const std::string& remote_ip, uint16_t remote_port);
    virtual ~MuxSocket();

public:
    bool Start() override {
        return true;
    }

    bool Stop() override {
        Close();
        return true;
    }

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

    inline bool CheckListener() override {
        return false;
    }
    void RegisterOnRecvCallback(callback_recv_t callback) override;

protected:
    // handle recv data, rewrite this function of yourself
    virtual int32_t HandleRecvData(const PacketPtr& packet);
    int32_t SendBinary(const uint8_t *data, uint32_t size);

protected:
    int fd_ { -1 };
    std::string local_ip_;
    uint16_t local_port_ { 0 };
    std::string remote_ip_;
    uint16_t remote_port_ { 0 };
    bool closed_ { true };
    callback_recv_t callback_;
    RingBuffer in_buf_ { RINGBUF_IN_SIZE };  // recv data buffer
    RingBuffer out_buf_{ RINGBUF_OUT_SIZE }; // send data buffer
};

} // end namespace transport

} // end namespace mux



