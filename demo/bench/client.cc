#include <unistd.h>

#include <thread>
#include <memory>
#include <string>
#include <iostream>
#include <atomic>

#include "transport/tcp_transport.h"
#include "mbase/mux_log.h"

using namespace mux;

int main(int argc, char* argv[]) {
    spdlog::set_level(spdlog::level::off); // close the log
    // Set the default logger to file logger
    auto file_logger = spdlog::basic_logger_mt("basic_logger", "log/client.log");
    spdlog::set_default_logger(file_logger);
    file_logger->flush_on(spdlog::level::err);

    MUX_DEBUG("log init");

    std::string server_ip {"127.0.0.1"};
    uint16_t server_port { 6666 };
    bool is_server = false;
    transport::TcpTransportPtr tcp_client = std::make_shared<transport::TcpTransport>(server_ip, server_port, is_server);
    if (!tcp_client) {
        std::cout << "tcp_client create faield!" << std::endl;
        MUX_ERROR("tcp_client create failed");
        exit(-1);
    }


    std::atomic<uint32_t> recv_num {0};
    auto recv_call = [&](const transport::SocketDataPtr& data) -> void {
        /*
        ::write(1, "\nrecv:", 6);
        ::write(1, data->msg_.data(), data->msg_.size());
        ::write(1, "\n", 1);
        */
        recv_num += 1;
        return;
    };

    tcp_client->RegisterOnRecvCallback(recv_call);

    if (!tcp_client->Start()) {
        std::cout << "tcp_client start failed!" << std::endl;
        MUX_ERROR("tcp_client start failed!");
        exit(1);
    }
    MUX_INFO("############tcp_client started!################");
    std::cout << "############tcp_client started! connected to ["<< server_ip << ":" << server_port << "] ################\n" << std::endl;
    std::cout << "begin bench test..." << std::endl;

    uint32_t send_total = 1000000;
    auto start = std::chrono::system_clock::now();
    std::string msg('b', 200);
    auto data = std::make_shared<transport::SocketData>();
    data->msg_ = msg;
    //for (uint32_t i = 0; i < send_total; ++i) {
    while (true) {
        tcp_client->SendData(data);
        //std::this_thread::sleep_for(std::chrono::microseconds(1)); // ms
    }
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "send "<< send_total << " packets,time takes:" << diff.count() << std::endl;
    std::cout << "recv total:" << recv_num << std::endl;

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
