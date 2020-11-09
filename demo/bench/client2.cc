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

std::atomic<uint32_t> conn { 0 };

bench::BenchTcpClient* create_client(const std::string& server_ip, uint16_t server_port) {
    bench::BenchTcpClient* tcp_client = new bench::BenchTcpClient(server_ip, server_port);
    if (!tcp_client) {
        std::cout << "tcp_client create faield!" << std::endl;
        MUX_ERROR("tcp_client create failed");
        return nullptr;
    }

    std::atomic<uint32_t> recv_num {0};
    auto recv_call = [&](const PacketPtr& packet) -> void {
        recv_num += 1;
        return;
    };

    tcp_client->RegisterOnRecvCallback(recv_call);

    /*
    // create and init EventTrigger
    int ep_num = 1;
    std::shared_ptr<transport::EventTrigger> event_trigger = std::make_shared<transport::EventTrigger>(ep_num);
    event_trigger->Start();
    */

    if (!tcp_client->Start()) {
        std::cout << "tcp_client start failed!" << std::endl;
        MUX_ERROR("tcp_client start failed!");
        return nullptr;
    }

    /*
    event_trigger->RegisterDescriptor((void*)tcp_client);
    */
    //std::cout << "############tcp_client started! connected to ["<< server_ip << ":" << server_port << "] ################\n" << std::endl;
    MUX_INFO("############tcp_client started!################");
    conn += 1;
    return tcp_client;
}

void multi_create_client(uint32_t clients, const std::string& server_ip, uint16_t server_port) {
    std::vector<bench::BenchTcpClient*> clients_vec;
    for (uint32_t i = 0; i < clients; ++i) {
        bench::BenchTcpClient* new_client = create_client(server_ip, server_port);
        if (new_client) {
            clients_vec.push_back(new_client);
        }
    }

    //std::cout << "successfully create " << clients_vec.size() << " clients" << std::endl;

    /*
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
    */

    std::this_thread::sleep_for(std::chrono::seconds(2));
    for (auto& client : clients_vec) {
        //std::cout << "delete client:" << client->GetLocalIp() << ":" << client->GetLocalPort() << std::endl;
        delete client;
    }
}


void run(const std::string& server_ip, uint16_t server_port, uint32_t clients, uint32_t threads, uint32_t loop) {
    //for (uint32_t n = 0; n < 100; ++n) {
    for (uint32_t n = 0; n < loop; ++n) {
        auto step_clients = clients / threads;
        auto left_clients = clients % threads;
        if (left_clients > 0) {
            threads += 1;
        }
        for (uint32_t i = 0; i < threads; ++i) {
            //std::cout << "start thread:" << i << std::endl;
            auto clients_num = step_clients;
            if (i == threads -1 && left_clients > 0) {
                clients_num = left_clients;
            }
            auto th = std::thread(multi_create_client, clients_num, server_ip, server_port);
            th.detach();
        }
        //std::cout << "start all thread done" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    std::cout << "done" << std::endl;
}



int main(int argc, char* argv[]) {
    if (argc <= 4) {
        std::cout << "usage: ./bench_client_accept ip port connections threads llop" << std::endl;
        std::cout << "usage: ./bench_client_accept 127.0.0.1 10000 30000 100 1" << std::endl;
	return 0;
    }
    // Set the default logger to file logger
    auto file_logger = spdlog::basic_logger_mt("basic_logger", "log/bench_client_accept.log");
    spdlog::set_default_logger(file_logger);
    //spdlog::set_level(spdlog::level::off); // close log
    spdlog::set_level(spdlog::level::debug); // close log
    file_logger->flush_on(spdlog::level::debug);
    MUX_DEBUG("log init");

    std::string server_ip {"127.0.0.1"};
    uint16_t server_port { 10000 };
    uint32_t clients = 1; // client number
    uint32_t threads = 1; // thread number
    if (argc >= 2) {
        server_ip = std::string(argv[1]);
    }
    if (argc >= 3) {
        server_port = std::atoi(argv[2]);
    }
    if (argc >= 4) {
        clients = std::atoi(argv[3]);
    }
    if (argc >= 5) {
        threads = std::atoi(argv[4]);
    }
    uint32_t loop = 1;
    if (argc >= 6) {
        loop = std::atoi(argv[5]);
    }

    auto th = std::thread(run, server_ip, server_port, clients, threads, loop);
    th.detach();

    while (true) {
        uint32_t old_conn = conn.load();

        auto start = std::chrono::system_clock::now();
        std::this_thread::sleep_for(std::chrono::seconds(5));
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double, std::micro> diff = end - start; // us

        uint32_t step_conn = conn.load() - old_conn;
        double rate = step_conn/ diff.count() * 1000 * 1000;
        std::cout << "total:" << conn << " step_recv:" << step_conn << " diff:" << diff.count() << " us" << " rate:" << rate << " tps" << std::endl;
    }


    return 0;
}
