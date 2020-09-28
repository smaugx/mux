#pragma once

#include <string>
#include <memory>

#include "mbase/include/mux_utils.h"
#include "mbase/protobuf/mux.pb.h"

namespace mux {

// packet header size is 2 bytes
typedef struct packet_header {
    uint16_t packet_len;
    uint16_t binary_protocol;  // json; protobuf; etc...
    uint32_t priority;        // message priority
} packet_header;

#define PACKET_LEN_MAX  32768  // 30KB

using PMessage = protobuf::MuxMessage;
using PMessagePtr = std::shared_ptr<PMessage>;

class Packet {
public:
    Packet();
    Packet(const std::string& body);
    Packet(const std::string& body, uint32_t priority);
    Packet(const std::string& body, uint16_t binary_protocol);
    Packet(const std::string& body, uint16_t binary_protocol, uint32_t priority);
    Packet(const PMessage& protobuf_msg);

public:
    packet_header& header();
    std::string& body();
    void set_from_ip_addr(const std::string& from_ip_addr);
    void set_from_ip_port(uint16_t from_ip_port);
    void set_to_ip_addr(const std::string& to_ip_addr);
    void set_to_ip_port(uint16_t to_ip_port);

public:
    const std::string& get_from_ip_addr();
    uint16_t get_from_ip_port();
    const std::string& get_to_ip_addr();
    uint16_t get_to_ip_port();
    uint16_t get_binary_protocol();
    uint32_t get_priority();

    template<typename T>
    std::shared_ptr<T> GetMessage();



private:
    packet_header header_;
    std::string   body_; // binary msg using protobuf，json or else protocol generated
    std::string  from_ip_addr_;
    uint16_t     from_ip_port_ { 0 };
    std::string  to_ip_addr_;
    uint16_t     to_ip_port_ { 0 };
};


typedef std::shared_ptr<Packet> PacketPtr;

}
