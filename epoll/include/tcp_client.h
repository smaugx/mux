#pragma once

#include <string>
#include <memory>
#include <thread>

#include "epoll/include/socket_imp.h"

namespace mux {

namespace transport {

class TcpClient : public MuxSocket {
public:
    TcpClient()                                  = delete;
    TcpClient(const TcpClient& other)            = delete;
    TcpClient& operator=(const TcpClient& other) = delete;
    TcpClient(TcpClient&& other)                 = delete;
    TcpClient& operator=(TcpClient&& other)      = delete;

    virtual ~TcpClient();
    TcpClient(const std::string& server_ip, uint16_t server_port);

public:
    bool Start() override;

protected:
    int32_t HandleRecvData(const PacketPtr& packet) override;

protected:
    int32_t CreateSocket();
    int32_t Connect(int32_t listenfd);
    int32_t MakeSocketNonBlock(int32_t fd);

private:
};

typedef std::shared_ptr<TcpClient> TcpClientPtr;

}

}
