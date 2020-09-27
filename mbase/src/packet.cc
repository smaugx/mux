#include "mbase/include/packet.h"

namespace mux {

namespace transport {

Packet::Packet() {
}

Packet::Packet(const std::string& msg)
    : msg { msg } {
    header.packet_len = msg.size();
}

Packet::Packet(uint32_t priority, const std::string& msg)
    : priority { priority },
      msg { msg } {
    header.packet_len = msg.size();
}

}

}
