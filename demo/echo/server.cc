#include <thread>
#include <memory>
#include <string>
#include <iostream>

#include "echo_tcp_acceptor.h"

#include "message_handle/include/message_handler.h"
#include "mbase/include/mux_log.h"
#include "socket/include/event_trigger.h"

using namespace mux;

int main(int argc, char* argv[]) {
    spdlog::set_level(spdlog::level::debug); // Set global log level to debug
    // Set the default logger to file logger
    auto file_logger = spdlog::basic_logger_mt("basic_logger", "log/echo_server.log");
    spdlog::set_default_logger(file_logger);
    file_logger->flush_on(spdlog::level::debug);

    MUX_DEBUG("log init");
    MUX_WARN("log init");


    std::string local_ip {"127.0.0.1"};
    uint16_t local_port { 6666 };
    if (argc >= 2) {
        local_ip = std::string(argv[1]);
    }
    if (argc >= 3) {
        local_port = std::atoi(argv[2]);
    }

    // create and init TcpAcceptor
    echo::EchoTcpAcceptor* echo_tcp_acceptor = new echo::EchoTcpAcceptor(local_ip, local_port);
    if (!echo_tcp_acceptor) {
        MUX_ERROR("echo_tcp_acceptor create failed");
        std::cout << "echo_tcp_acceptor create failed" << std::endl;
        exit(-1);
    }

    auto recv_call = [&](const transport::PacketPtr& packet) -> void {
        auto key = packet->from_ip_addr + ":" + std::to_string(packet->from_ip_port);
        auto sock = echo_tcp_acceptor->FindSession(key);
        if (!sock) {
            MUX_WARN("not found session:{0}", key);
            return;
        }
        sock->SendData(packet);
        return;
    };

    std::shared_ptr<transport::MessageHandler> msg_handle = std::make_shared<transport::MessageHandler>();
    msg_handle->Init();
    msg_handle->RegisterOnDispatchCallback(recv_call);
    auto dispath_call = [&](transport::PacketPtr& packet) -> void {
        return msg_handle->HandleMessage(packet);
    };


    echo_tcp_acceptor->RegisterNewSocketRecvCallback(dispath_call);


    // create and init EventTrigger
    int ep_num = 1;
    std::shared_ptr<transport::EventTrigger> event_trigger = std::make_shared<transport::EventTrigger>(ep_num);
    auto accept_callback = [&](int32_t cli_fd, const std::string& remote_ip, uint16_t remote_port) -> transport::BasicSocket* {
        return echo_tcp_acceptor->OnSocketAccept(cli_fd, remote_ip, remote_port);
    };
    event_trigger->Start();

    if (!echo_tcp_acceptor->Start()) {
        MUX_ERROR("echo_tcp_acceptor start failed");
        std::cout << "echo_tcp_acceptor start failed" << std::endl;
        exit(1);
    }

    // attention: RegisterDescriptor must after Start
    event_trigger->RegisterOnAcceptCallback(accept_callback);
    event_trigger->RegisterDescriptor((void*)echo_tcp_acceptor);

    std::cout << "############tcp_echo_server[" << echo_tcp_acceptor->GetLocalIp() << ":" << echo_tcp_acceptor->GetLocalPort()  << "] started!################\n" << std::endl;
    MUX_INFO("############tcp_echo_server [{0}:{1}] started!################", local_ip, local_port);

    while (true) {
        std::cout << std::endl<<  "input:";
        std::string msg;
        std::getline(std::cin, msg);
        if (msg.compare("q") == 0 || msg.compare("quit") == 0 || msg.compare("exit") == 0) {
            break;
        }
        //std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "exit, wait clean..." << std::endl;
    event_trigger->Stop();
    delete echo_tcp_acceptor;
    msg_handle->Join();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 0;
}
