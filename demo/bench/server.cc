#include <thread>
#include <memory>
#include <string>
#include <iostream>

#include "bench_tcp_acceptor.h"

#include "message_handle/include/message_handler.h"
#include "mbase/include/mux_log.h"
#include "socket/include/event_trigger.h"
#include "mbase/include/packet.h"

// nlohmann_json
#include <nlohmann/json.hpp>
using json = nlohmann::json;

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
    if (argc >= 2) {
        local_ip = std::string(argv[1]);
    }
    if (argc >= 3) {
        local_port = std::atoi(argv[2]);
    }

    // create and init TcpAcceptor
    bench::BenchTcpAcceptor* bench_tcp_acceptor = new bench::BenchTcpAcceptor(local_ip, local_port);
    if (!bench_tcp_acceptor) {
        MUX_ERROR("echo_tcp_acceptor create failed");
        std::cout << "echo_tcp_acceptor create failed" << std::endl;
        exit(-1);
    }


    std::atomic<uint32_t> recv_num {0};
    auto recv_call = [&](const PacketPtr& packet) -> void {
        //tcp_bench_server->SendData(packet);
        recv_num += 1;
        return;
    };


    std::shared_ptr<transport::MessageHandler> msg_handle = std::make_shared<transport::MessageHandler>();
    msg_handle->Init();
    msg_handle->RegisterOnDispatchCallback(recv_call);
    auto dispath_call = [&](PacketPtr& packet) -> void {
        return msg_handle->HandleMessage(packet);
    };
    bench_tcp_acceptor->RegisterNewSocketRecvCallback(dispath_call);

    // create and init EventTrigger
    int ep_num = 4;
    std::shared_ptr<transport::EventTrigger> event_trigger = std::make_shared<transport::EventTrigger>(ep_num);
    auto accept_callback = [&](int32_t cli_fd, const std::string& remote_ip, uint16_t remote_port) -> transport::BasicSocket* {
        return bench_tcp_acceptor->OnSocketAccept(cli_fd, remote_ip, remote_port);
    };
    event_trigger->Start();

    if (!bench_tcp_acceptor->Start()) {
        MUX_ERROR("echo_tcp_acceptor start failed");
        std::cout << "echo_tcp_acceptor start failed" << std::endl;
        exit(1);
    }

    // attention: RegisterDescriptor must after Start
    event_trigger->RegisterOnAcceptCallback(accept_callback);
    event_trigger->RegisterDescriptor((void*)bench_tcp_acceptor);

    std::cout << "############tcp_echo_server[" << bench_tcp_acceptor->GetLocalIp() << ":" << bench_tcp_acceptor->GetLocalPort()  << "] started!################\n" << std::endl;
    MUX_INFO("############tcp_echo_server [{0}:{1}] started!################", local_ip, local_port);



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
    std::cout << "exit, wait clean..." << std::endl;
    event_trigger->Stop();
    delete bench_tcp_acceptor;
    msg_handle->Join();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    return 0;
}
