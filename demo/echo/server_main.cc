#include <thread>
#include <memory>
#include <string>
#include <iostream>

#include "transport/tcp_transport.h"
#include "mbase/mux_log.h"

using namespace mux;

int main(int argc, char* argv[]) {
    spdlog::set_level(spdlog::level::debug); // Set global log level to debug
    // Set the default logger to file logger
    auto file_logger = spdlog::basic_logger_mt("basic_logger", "log/echo_server.log");
    spdlog::set_default_logger(file_logger);
    file_logger->flush_on(spdlog::level::warn);

    MUX_DEBUG("log init");


    std::string local_ip {"127.0.0.1"};
    uint16_t local_port { 6666 };
    bool is_server = true;
    transport::TcpTransportPtr tcp_echo_server = std::make_shared<transport::TcpTransport>(local_ip, local_port, is_server);
    if (!tcp_echo_server) {
        std::cout << "tcp_echo_server create faield!" << std::endl;
        MUX_ERROR("tcp_echo_server create failed");
        exit(-1);
    }

    auto recv_call = [&](const transport::SocketDataPtr& data) -> void {
        tcp_echo_server->SendData(data);
        return;
    };

    tcp_echo_server->RegisterOnRecvCallback(recv_call);

    if (!tcp_echo_server->Start()) {
        MUX_ERROR("tcp_echo_server start failed!");
        std::cout << "tcp_echo_server start failed!" << std::endl;
        exit(1);
    }
    std::cout << "############tcp_echo_server started!################" << std::endl;
    MUX_INFO("############tcp_echo_server started!################");

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
