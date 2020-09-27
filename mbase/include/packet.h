#pragma once

#include <string>
#include <memory>

#include "mbase/include/mux_utils.h"

namespace mux {

namespace transport {

// packet header size is 2 bytes
typedef struct packet_header {
    uint16_t packet_len;
} packet_header;

#define PACKET_HEADER_SIZE sizeof((packet_header))
#define PACKET_LEN_MAX  32768  // 30KB

typedef struct Packet {
public:
    Packet();
    Packet(const std::string& msg);
    Packet(uint32_t priority, const std::string& msg);

    packet_header header;
    uint32_t priority {kMaxPacketPriority}; // default is the lowest priority
    std::string msg;
    std::string from_ip_addr;
    uint16_t    from_ip_port { 0 };
    std::string to_ip_addr;
    uint16_t    to_ip_port { 0 };
} Packet;

typedef std::shared_ptr<Packet> PacketPtr;
}

}
