#pragma once

#include <string>
#include <memory>

#include "mbase/mux_utils.h"

namespace mux {

namespace transport {

typedef struct Packet {
public:
    Packet();
    Packet(const std::string& msg);
    Packet(uint32_t priority, const std::string& msg);

    uint32_t    priority_ {kMaxPacketPriority}; // default is the lowest priority
    std::string msg_;
    std::string from_ip_addr_;
    uint16_t    from_ip_port_;
    std::string to_ip_addr_;
    uint16_t    to_ip_port_;
} Packet;

typedef std::shared_ptr<Packet> PacketPtr;

}

}
