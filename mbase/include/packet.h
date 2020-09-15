#pragma once

#include <string>
#include <memory>

#include "mbase/include/mux_utils.h"

namespace mux {

namespace transport {

typedef struct Packet {
public:
    Packet();
    Packet(const std::string& msg);
    Packet(uint32_t priority, const std::string& msg);

    uint32_t priority {kMaxPacketPriority}; // default is the lowest priority
    std::string msg;
} Packet;

typedef std::shared_ptr<Packet> PacketPtr;
}

}
