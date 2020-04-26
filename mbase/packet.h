#pragma once

#include <string>
#include <memory>

#include "mbase/mux_utils.h"

namespace mux {

namespace transport {

typedef struct Packet {
public:
    Packet() {}
    Packet(int32_t fd, const std::string& msg)
        : fd_ { fd },
          msg_ { msg } {}
    Packet(int32_t fd, uint32_t priority, const std::string& msg)
        : fd_ { fd },
          priority_ {priority},
          msg_ { msg } {}

    int32_t fd_ { -1 };
    uint32_t priority_ {kMaxPacketPriority}; // default is the lowest priority
    std::string msg_;
} Packet;

typedef std::shared_ptr<Packet> PacketPtr;

using callback_recv_t = std::function<void(transport::PacketPtr&)>;

}

}
