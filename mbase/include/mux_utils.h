#pragma once

#include <memory>
#include <string>
#include <functional>

namespace mux {

namespace transport {

class BasicSocket;

}

class Packet;
using PacketPtr = std::shared_ptr<Packet>;



enum PacketBinaryProtocol {
    kInvalidBinaryProtocol = 0,
    kProtobufBinaryProtocol,
    kJsonBinaryProtocol
};

static const uint32_t kMaxPacketPriority = 3;
// the lowest, the highest priority
enum PacketPriority {
    kCriticalType = 0,
    kHighType,
    kMediumType,
    kLowType,
};


using callback_recv_t    = std::function<void(PacketPtr&)>;
using callback_accept_t  = std::function<transport::BasicSocket*(int32_t, const std::string&, uint16_t)>;
using callback_sockerr_t = std::function<void(transport::BasicSocket*)>;

}
