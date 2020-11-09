#pragma once

#include <string>
#include <memory>
#include <thread>
#include <vector>
#include <mutex>

#include "socket/include/socket_imp.h"
#include "mbase/include/mux_utils.h"

namespace mux {

namespace transport {

class SocketBase;

class TcpAcceptor : public SocketBase {
public:
    TcpAcceptor()                                    = default;
    TcpAcceptor(const TcpAcceptor& other)            = delete;
    TcpAcceptor& operator=(const TcpAcceptor& other) = delete;
    TcpAcceptor(TcpAcceptor&& other)                 = delete;
    TcpAcceptor& operator=(TcpAcceptor&& other)      = delete;
    virtual ~TcpAcceptor();

    TcpAcceptor(const std::string& local_ip, uint16_t local_port);

public:
    bool Start() override;
    bool Stop() override;

    inline std::string GetLocalIp() override {
        return local_ip_;
    }

    inline uint16_t GetLocalPort() override  {
        return local_port_;
    }

    inline int32_t GetDescriptor() override {
        return handle_;
    }

    inline bool CheckListener() override {
        return true;
    }
    void Close() override;

public:
    void RegisterNewSocketRecvCallback(callback_recv_t callback);
    virtual BasicSocket* OnSocketAccept(int32_t cli_fd, const std::string& remote_ip, uint16_t remote_port);
    virtual void OnSocketErr(BasicSocket* sock);

protected:
    int32_t CreateSocket();
    int32_t MakeSocketNonBlock(int32_t fd);
    int32_t Listen(int32_t listenfd);
    void ClearConnections();
    BasicSocket* ManageNewConnection(BasicSocket* new_sock);


protected:
    std::string local_ip_;
    uint16_t local_port_ { 0 };
    int32_t handle_ { -1 }; // listenfd
    std::mutex connection_vec_mutex_;
    std::vector<BasicSocket*> connection_vec_;
    callback_recv_t new_socket_recv_callback_;
};


typedef std::shared_ptr<TcpAcceptor> TcpAcceptorPtr;

}

}
