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

int main(int argc, char* argv[]) {
    spdlog::set_level(spdlog::level::warn); // Set global log level to debug
    // Set the default logger to file logger
    auto file_logger = spdlog::basic_logger_mt("basic_logger", "log/client.log");
    spdlog::set_default_logger(file_logger);
    file_logger->flush_on(spdlog::level::warn);

    MUX_DEBUG("log init");

    std::string server_ip {"127.0.0.1"};
    uint16_t server_port { 6666 };
    if (argc >= 2) {
        server_ip = std::string(argv[1]);
    }
    if (argc >= 3) {
        server_port = std::atoi(argv[2]);
    }

    bench::BenchTcpClient* tcp_client = new bench::BenchTcpClient(server_ip, server_port);
    if (!tcp_client) {
        std::cout << "tcp_client create faield!" << std::endl;
        MUX_ERROR("tcp_client create failed");
        exit(-1);
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
        exit(1);
    }

    event_trigger->RegisterDescriptor((void*)tcp_client);
    std::cout << "############tcp_client started! connected to ["<< server_ip << ":" << server_port << "] ################\n" << std::endl;
    MUX_INFO("############tcp_client started!################");

    std::vector<uint32_t> body_size_list = {200, 500}; // 200B, 500B, 1024B, 1500B...
    for (auto item : body_size_list) {
        std::cout << "benchmark: packet body size = " << item << std::endl;
        std::string msg(item, 'b');
        PMessage pmsg;
        pmsg.set_data(msg);
        auto packet = std::make_shared<Packet>(pmsg);
        uint32_t send_num = 0;
        while (send_num < 5000000) {
            uint16_t random_priority = send_num % (mux::kMaxPacketPriority +1);
            packet->set_priority(random_priority);
            tcp_client->SendData(packet);
            //std::this_thread::sleep_for(std::chrono::microseconds(1)); // ms

            send_num += 1;
        }

        std::cout << "benchmark: packet body size = " << item << " done. (sleep 10 seconds for next benchmark)"<< std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    std::cout << "all benchmark done" << std::endl;
    event_trigger->Stop();
    delete tcp_client;

    std::cout << "exit, wait clean..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 0;
}
