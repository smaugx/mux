#pragma once

#include <string>
#include <memory>
#include <vector>

#include "mbase/include/mux_utils.h"
#include "mbase/protobuf/mux.pb.h"

namespace mux {

// packet header size is 2 bytes
typedef struct packet_header {
    uint16_t packet_len;
    uint8_t binary_protocol;  // json; protobuf; etc...
    uint8_t priority;        // message priority
} packet_header;

#define PACKET_HEAD_SIZE (sizeof(packet_header))
#define PACKET_LEN_MAX  32768 // 32KB

using PMessage = protobuf::MuxMessage;
using PMessagePtr = std::shared_ptr<PMessage>;

class Packet {
public:
    Packet();
    Packet(const std::string& body);
    Packet(const std::string& body, uint8_t binary_protocol, uint8_t priority);
    Packet(const PMessage& protobuf_msg);
    Packet(const uint8_t* data, uint32_t size); // header + body

public:
    void set_from_ip_addr(const std::string& from_ip_addr);
    void set_from_ip_port(uint16_t from_ip_port);
    void set_to_ip_addr(const std::string& to_ip_addr);
    void set_to_ip_port(uint16_t to_ip_port);
    uint16_t append_body(const std::string& data);
    void set_priority(uint16_t priority);

public:
    const uint8_t* data() const; // header + body
    uint32_t size() const; // header size + body size
    const uint8_t* body() const; // body
    uint16_t body_size() const; // body size
    const std::string& get_from_ip_addr();
    uint16_t get_from_ip_port();
    const std::string& get_to_ip_addr();
    uint16_t get_to_ip_port();
    uint16_t get_binary_protocol();
    uint16_t get_priority();

    template<typename T>
    std::shared_ptr<T> GetMessage();
private:
    std::vector<uint8_t> data_; // binary msg using protobuf，json or else protocol generated
    std::string  from_ip_addr_;
    uint16_t     from_ip_port_ { 0 };
    std::string  to_ip_addr_;
    uint16_t     to_ip_port_ { 0 };
};


typedef std::shared_ptr<Packet> PacketPtr;

}
