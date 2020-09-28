#include <unistd.h>

#include <thread>
#include <memory>
#include <string>
#include <iostream>

#include "echo_client.h"
#include "mbase/include/mux_log.h"
#include "socket/include/event_trigger.h"
#include "mbase/include/packet.h"
#include "mbase/protobuf/mux.pb.h"


using namespace mux;

int main(int argc, char* argv[]) {
    spdlog::set_level(spdlog::level::debug); // Set global log level to debug
    // Set the default logger to file logger
    auto file_logger = spdlog::basic_logger_mt("basic_logger", "log/client.log");
    spdlog::set_default_logger(file_logger);
    file_logger->flush_on(spdlog::level::debug);

    MUX_DEBUG("log init");

    std::string server_ip {"127.0.0.1"};
    uint16_t server_port { 6666 };
    if (argc >= 2) {
        server_ip = std::string(argv[1]);
    }
    if (argc >= 3) {
        server_port = std::atoi(argv[2]);
    }

    echo::EchoTcpClient* tcp_client = new echo::EchoTcpClient(server_ip, server_port);
    if (!tcp_client) {
        std::cout << "tcp_client create faield!" << std::endl;
        MUX_ERROR("tcp_client create failed");
        exit(-1);
    }

    auto recv_call = [](const PacketPtr& packet) -> void {
        PMessagePtr message = packet->GetMessage<PMessage>();
        if (!message) {
            return;
        }
        std::cout << "\nrecv:" << message->data() << std::endl << std::endl;
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

    /*
    while (true) {
        auto packet = std::make_shared<transport::Packet>();
        packet->msg = std::string(200, 'b');
        tcp_client->SendData(packet);
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    */

    while (true) {
        std::cout << std::endl<<  "input:";
        std::string msg;
        std::getline(std::cin, msg);
        if (msg.compare("q") == 0 || msg.compare("quit") == 0 || msg.compare("exit") == 0) {
            break;
        }
        if (msg.empty()) {
            continue;
        }
        PMessage pmsg;
        pmsg.set_data(msg);
        auto packet = std::make_shared<Packet>(pmsg);
        tcp_client->SendData(packet);
        //std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    event_trigger->Stop();

    delete tcp_client;

    std::cout << "exit, wait clean..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 0;
}
