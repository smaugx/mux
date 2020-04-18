#include <thread>
#include <memory>
#include <string>
#include <iostream>

#include "tcp_transport.h"

using namespace mux;

int main(int argc, char* argv[]) {
    std::string server_ip {"127.0.0.1"};
    uint16_t server_port { 6666 };
    bool is_server = false;
    transport::TcpTransportPtr tcp_client = std::make_shared<transport::TcpTransport>(server_ip, server_port, is_server);
    if (!tcp_client) {
        std::cout << "tcp_client create faield!" << std::endl;
        exit(-1);
    }


    auto recv_call = [](const transport::SocketDataPtr& data) -> void {
        std::cout << "in recv call: fd:" << data->fd_ << " msg.size:" << data->msg_.size() << std::endl;
        return;
    };

    tcp_client->RegisterOnRecvCallback(recv_call);

    if (!tcp_client->Start()) {
        std::cout << "tcp_client start failed!" << std::endl;
        exit(1);
    }
    std::cout << "############tcp_client started!################" << std::endl;

    uint32_t send_total = 1000000;
    auto start = std::chrono::system_clock::now();
    for (uint32_t i = 0; i < send_total; ++i) {
        std::string msg('c', 200);
        msg += std::to_string(i);
        tcp_client->SendData(0, msg);
    }
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "send "<< send_total << " packets,time takes:" << diff.count() << std::endl;

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
