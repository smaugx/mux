#pragma once

namespace mux {


enum PacketPriority {
    // the larger priority_num , the lower priority
    kHighPriority = 0,
    kMiddlePriority,
    kLowPriority,
};

static const uint32_t kMaxPacketPriority = 3;

}
