#include <thread>
#include <memory>
#include <string>
#include <iostream>

#include "transport/tcp_transport.h"

using namespace mux;

int main(int argc, char* argv[]) {
    std::string local_ip {"127.0.0.1"};
    uint16_t local_port { 6666 };
    bool is_server = true;
    transport::TcpTransportPtr tcp_echo_server = std::make_shared<transport::TcpTransport>(local_ip, local_port, is_server);
    if (!tcp_echo_server) {
        std::cout << "tcp_echo_server create faield!" << std::endl;
        exit(-1);
    }

    auto recv_call = [&](const transport::SocketDataPtr& data) -> void {
        tcp_echo_server->SendData(data);
        return;
    };

    tcp_echo_server->RegisterOnRecvCallback(recv_call);

    if (!tcp_echo_server->Start()) {
        std::cout << "tcp_echo_server start failed!" << std::endl;
        exit(1);
    }
    std::cout << "############tcp_echo_server started!################" << std::endl;

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
