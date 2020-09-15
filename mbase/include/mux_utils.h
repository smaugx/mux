#pragma once

#include <memory>
#include <string>
#include <functional>

namespace mux {

namespace transport {
struct Packet;
using PacketPtr = std::shared_ptr<Packet>;

class BasicSocket;
}


// the larger priority_num , the lower priority
// 0 is highest priority
static const uint32_t kMaxPacketPriority = 3;


using callback_recv_t    = std::function<void(transport::PacketPtr&)>;
using callback_accept_t  = std::function<transport::BasicSocket*(int32_t, const std::string&, uint16_t)>;

}
