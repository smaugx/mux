#include "mbase/packet.h"

namespace mux {

namespace transport {

Packet::Packet() {
}

Packet::Packet(int32_t fd, const std::string& msg)
    : fd_ { fd },
      msg_ { msg } {
}

Packet::Packet(int32_t fd, uint32_t priority, const std::string& msg)
    : fd_ { fd },
      priority_ { priority },
      msg_ { msg } {
}

}

}
