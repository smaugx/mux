#include <unistd.h>

#include <thread>
#include <memory>
#include <string>
#include <iostream>

#include "bench_client.h"
#include "mbase/include/mux_log.h"
#include "socket/include/event_trigger.h"
#include "mbase/include/packet.h"


using namespace mux;

int new_client(const std::string& server_ip, uint16_t server_port) {
    bench::BenchTcpClient* tcp_client = new bench::BenchTcpClient(server_ip, server_port);
    if (!tcp_client) {
        std::cout << "tcp_client create faield!" << std::endl;
        MUX_ERROR("tcp_client create failed");
        return -1;
    }

    std::atomic<uint32_t> recv_num {0};
    auto recv_call = [&](const PacketPtr& packet) -> void {
        recv_num += 1;
        return;
    };

    tcp_client->RegisterOnRecvCallback(recv_call);

    // create and init EventTrigger
    int ep_num = 1;
    std::shared_ptr<transport::EventTrigger> event_trigger = std::make_shared<transport::EventTrigger>(ep_num);
    event_trigger->Start();

    if (!tcp_client->Start()) {
        std::cout << "tcp_client start failed!" << std::endl;
        MUX_ERROR("tcp_client start failed!");
        return -1;
    }

    event_trigger->RegisterDescriptor((void*)tcp_client);
    std::cout << "############tcp_client started! connected to ["<< server_ip << ":" << server_port << "] ################\n" << std::endl;
    MUX_INFO("############tcp_client started!################");
    return 0;
}

int main(int argc, char* argv[]) {
    // Set the default logger to file logger
    auto file_logger = spdlog::basic_logger_mt("basic_logger", "log/client.log");
    spdlog::set_default_logger(file_logger);
    spdlog::set_level(spdlog::level::off); // close log
    MUX_DEBUG("log init");

    std::string server_ip {"127.0.0.1"};
    uint16_t server_port { 6666 };
    uint32_t threads = 1;
    if (argc >= 2) {
        server_ip = std::string(argv[1]);
    }
    if (argc >= 3) {
        server_port = std::atoi(argv[2]);
    }
    if (argc >= 4) {
        threads = std::atoi(argv[3]);
    }

    for (uint32_t i = 0; i < threads; ++i) {
        std::cout << "start thread:" << i << std::endl;
        auto th = std::thread(new_client, server_ip, server_port);
        th.detach();
    }
    std::cout << "start all thread done" << std::endl;

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
