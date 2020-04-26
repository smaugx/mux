#pragma once

#include <string>
#include <memory>

#include "mbase/mux_utils.h"

namespace mux {

namespace transport {

typedef struct Packet {
public:
    Packet()
        : fd_ { -1 },
          msg_ { "" } {}
    Packet(int32_t fd, const std::string& msg)
        : fd_ { fd },
          msg_ { msg } {}

    int32_t fd_ { -1 };
    uint32_t priority_ {kMaxPacketPriority}; // default is the lowest priority
    std::string msg_;
} Packet;

typedef std::shared_ptr<Packet> PacketPtr;

}

}
