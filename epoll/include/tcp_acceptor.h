#pragma once

#include <string>
#include <memory>
#include <thread>

namespace mux {

namespace transport {

class EventTrigger;

class TcpAcceptor {
public:
    TcpAcceptor()                                    = default;
    TcpAcceptor(const TcpAcceptor& other)            = delete;
    TcpAcceptor& operator=(const TcpAcceptor& other) = delete;
    TcpAcceptor(TcpAcceptor&& other)                 = delete;
    TcpAcceptor& operator=(TcpAcceptor&& other)      = delete;
    ~TcpAcceptor() override;

    TcpAcceptor(const std::string& local_ip, uint16_t local_port);

public:
    bool Start() override;
    bool Stop() override;
    int32_t SendData(const PacketPtr& packet) override;
    void RegisterOnRecvCallback(callback_recv_t callback) override;
    void UnRegisterOnRecvCallback() override;

protected:
    int32_t CreateSocket();
    int32_t MakeSocketNonBlock(int32_t fd);
    int32_t Listen(int32_t listenfd);
    void OnSocketAccept();
    void Close();


private:
    std::string local_ip_;
    uint16_t local_port_ { 0 };
    int32_t handle_ { -1 }; // listenfd
    std::weak_ptr<EventTrigger> evt_trigger_;
};


typedef std::shared_ptr<TcpAcceptor> TcpAcceptorPtr;

}

}
