#include "mbase/packet.h"

namespace mux {

namespace transport {

Packet::Packet() {
}

Packet::Packet(const std::string& msg)
    : msg_ { msg } {
}

Packet::Packet(uint32_t priority, const std::string& msg)
    : priority_ { priority },
      msg_ { msg } {
}

}

}
