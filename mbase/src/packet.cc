#include "mbase/include/packet.h"
#include "mbase/include/mux_log.h"

namespace mux {


Packet::Packet() {
}

Packet::Packet(const std::string& body)
    : body_{ body } {
    header_.packet_len = body.size();
    header_.binary_protocol = kInvalidBinaryProtocol;
    header_.priority = kLowType;
}

Packet::Packet(const std::string& body, uint32_t priority)
    : body_{ body } {
    header_.packet_len = body.size();
    header_.binary_protocol = kInvalidBinaryProtocol;
    header_.priority = priority;
}


Packet::Packet(const std::string& body, uint16_t binary_protocol)
    : body_{ body } {
    header_.packet_len = body.size();
    header_.binary_protocol = binary_protocol;
    header_.priority = kLowType;
}

Packet::Packet(const std::string& body, uint16_t binary_protocol, uint32_t priority)
    : body_{ body } {
    header_.packet_len = body.size();
    header_.binary_protocol = binary_protocol;
    header_.priority = priority;
}

Packet::Packet(const PMessage& protobuf_msg) {
    protobuf_msg.SerializeToString(&body_);
    header_.packet_len = body_.size();
    header_.binary_protocol = kProtobufBinaryProtocol;
    header_.priority = kLowType;
}

packet_header& Packet::header() {
    return header_;
}

std::string& Packet::body() {
    return body_;
}


const std::string& Packet::get_from_ip_addr() {
    return from_ip_addr_;
}

uint16_t Packet::get_from_ip_port() {
    return from_ip_port_;
}

const std::string& Packet::get_to_ip_addr() {
    return to_ip_addr_;
}

uint16_t Packet::get_to_ip_port() {
    return to_ip_port_;
}

uint16_t Packet::get_binary_protocol() {
    return header_.binary_protocol;
}

uint32_t Packet::get_priority() {
    return header_.priority;
}

void Packet::set_from_ip_addr(const std::string& from_ip_addr) {
    from_ip_addr_ = from_ip_addr;
}

void Packet::set_from_ip_port(uint16_t from_ip_port) {
    from_ip_port_ = from_ip_port;
}

void Packet::set_to_ip_addr(const std::string& to_ip_addr) {
    to_ip_addr_ = to_ip_addr;
}

void Packet::set_to_ip_port(uint16_t to_ip_port) {
    to_ip_port_ = to_ip_port;
}

template<>
PMessagePtr Packet::GetMessage<PMessage>() {
    if (header_.binary_protocol != kProtobufBinaryProtocol) {
        MUX_ERROR("GetMessage(protobuf) type failed:{0}", header_.binary_protocol);
        return nullptr;
    }

    PMessagePtr message = std::make_shared<PMessage>();
    if (!message->ParseFromArray(body_.data(), body_.size()))
    {
        MUX_ERROR("Message ParseFromString from string failed!");
        return nullptr;
    }
    return message;
}

}
