#include <thread>
#include <memory>
#include <string>
#include <iostream>

#include "transport/include/tcp_transport.h"
#include "mbase/packet.h"
#include "mbase/mux_log.h"

using namespace mux;

int main(int argc, char* argv[]) {
    spdlog::set_level(spdlog::level::off); // close the log
    // Set the default logger to file logger
    auto file_logger = spdlog::basic_logger_mt("basic_logger", "log/bench_server.log");
    spdlog::set_default_logger(file_logger);
    file_logger->flush_on(spdlog::level::err);

    MUX_DEBUG("log init");


    std::string local_ip {"127.0.0.1"};
    uint16_t local_port { 6666 };
    bool is_server = true;
    transport::TcpTransportPtr tcp_bench_server = std::make_shared<transport::TcpTransport>(local_ip, local_port, is_server);
    if (!tcp_bench_server) {
        std::cout << "tcp_bench_server create faield!" << std::endl;
        MUX_ERROR("tcp_bench_server create failed");
        exit(-1);
    }

    std::atomic<uint32_t> recv_num {0};
    auto recv_call = [&](const transport::PacketPtr& packet) -> void {
        //tcp_bench_server->SendData(packet);
        recv_num += 1;
        return;
    };

    tcp_bench_server->RegisterOnRecvCallback(recv_call);

    if (!tcp_bench_server->Start()) {
        MUX_ERROR("tcp_bench_server start failed!");
        std::cout << "tcp_bench_server start failed!" << std::endl;
        exit(1);
    }
    std::cout << "############tcp_bench_server ["<< local_ip << ":" << local_port << "] started!################" << std::endl;
    MUX_INFO("############tcp_bench_server [{0}:{1}] started!################", local_ip, local_port);


    while (true) {
        uint32_t old_recv = recv_num.load();

        auto start = std::chrono::system_clock::now();
        std::this_thread::sleep_for(std::chrono::seconds(5));
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double, std::micro> diff = end - start; // us

        uint32_t step_recv = recv_num.load() - old_recv;
        double rate = step_recv / diff.count() * 1000 * 1000;
        std::cout << "total:" << recv_num << " step_recv:" << step_recv << " diff:" << diff.count() << " us" << " rate:" << rate << " tps" << std::endl;

    }

    return 0;
}
