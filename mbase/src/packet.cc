#include "mbase/include/packet.h"
#include "mbase/include/mux_log.h"

#include <algorithm>

namespace mux {


Packet::Packet() {
    packet_header header;
    header.packet_len = 0;
    header.binary_protocol = kInvalidBinaryProtocol;
    header.priority = kLowType;
    /*
    for (uint32_t i = 0; i < PACKET_HEAD_SIZE; ++i) {
        data_.push_back(*((uint8_t*)(&header + i)));
    }
    */
    std::copy_n((uint8_t*)&header, PACKET_HEAD_SIZE, std::back_inserter(data_));
}

Packet::Packet(uint16_t capacity) {
    packet_header header;
    header.packet_len = 0;
    header.binary_protocol = kInvalidBinaryProtocol;
    header.priority = kLowType;
    /*
    for (uint32_t i = 0; i < PACKET_HEAD_SIZE; ++i) {
        data_.push_back(*((uint8_t*)(&header + i)));
    }
    */
    std::copy_n((uint8_t*)&header, PACKET_HEAD_SIZE, std::back_inserter(data_));
    data_.resize(capacity);
}

Packet::Packet(const std::string& body) {
    packet_header header;
    header.packet_len = body.size();
    header.binary_protocol = kInvalidBinaryProtocol;
    header.priority = kLowType;
    /*
    for (uint32_t i = 0; i < PACKET_HEAD_SIZE; ++i) {
        data_.push_back(*((uint8_t*)(&header + i)));
    }
    */
    std::copy_n((uint8_t*)&header, PACKET_HEAD_SIZE, std::back_inserter(data_));
    std::copy(body.begin(), body.end(), std::back_inserter(data_));
}

Packet::Packet(const std::string& body, uint8_t binary_protocol, uint8_t priority) {
    packet_header header;
    header.packet_len = body.size();
    header.binary_protocol = binary_protocol;
    header.priority = priority;
    /*
    for (uint32_t i = 0; i < PACKET_HEAD_SIZE; ++i) {
        data_.push_back(*((uint8_t*)(&header + i)));
    }
    */
    std::copy_n((uint8_t*)&header, PACKET_HEAD_SIZE, std::back_inserter(data_));
    std::copy(body.begin(), body.end(), std::back_inserter(data_));
}

Packet::Packet(const PMessage& protobuf_msg) {
    std::string body;
    protobuf_msg.SerializeToString(&body);

    packet_header header;
    header.packet_len = body.size();
    header.binary_protocol = kProtobufBinaryProtocol;
    header.priority = kLowType;
    std::copy_n((uint8_t*)&header, PACKET_HEAD_SIZE, std::back_inserter(data_));

    /*
    packet_header assert_header;
    memcpy(&assert_header, data_.data(), PACKET_HEAD_SIZE);
    std::cout << "assert_header.packet_len = " << (uint32_t)assert_header.packet_len << std::endl;
    std::cout << "assert_header.binary_protocol = " << (uint32_t)assert_header.binary_protocol << std::endl;
    std::cout << "assert_header.priority = " << (uint32_t)assert_header.priority << std::endl;
    assert(assert_header.packet_len ==  body.size());
    assert(assert_header.binary_protocol == kProtobufBinaryProtocol);
    assert(assert_header.priority == kLowType);
    */

    std::copy(body.begin(), body.end(), std::back_inserter(data_));
}

Packet::Packet(const uint8_t* data, uint32_t size) {
    std::copy_n(data, size, std::back_inserter(data_));

    /*
    std::string body_data((char*)body(), body_size());
    packet_header assert_header;
    memcpy(&assert_header, data_.data(), PACKET_HEAD_SIZE);
    std::cout << "assert_header.packet_len = " << (uint32_t)assert_header.packet_len << std::endl;
    std::cout << "assert_header.binary_protocol = " << (uint32_t)assert_header.binary_protocol << std::endl;
    std::cout << "assert_header.priority = " << (uint32_t)assert_header.priority << std::endl;
    std::cout << "body:" << body_data << std::endl;
    */
}

const uint8_t* Packet::data() const {
    return data_.data();
}

uint32_t Packet::size() const {
    return data_.size();
}

const uint8_t* Packet::body() const {
    return data_.data() + PACKET_HEAD_SIZE;
}

uint16_t Packet::body_size() const {
    return data_.size() - PACKET_HEAD_SIZE;
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
    packet_header header;
    memcpy(&header, data_.data(), PACKET_HEAD_SIZE);
    return (uint16_t)header.binary_protocol;
}

uint16_t Packet::get_priority() {
    packet_header header;
    memcpy(&header, data_.data(), PACKET_HEAD_SIZE);
    return (uint16_t)header.priority;
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

void Packet::set_priority(uint16_t priority) {
    packet_header tmp_header;
    memcpy(&tmp_header, data_.data(), PACKET_HEAD_SIZE);
    tmp_header.priority = priority;
    std::copy_n((uint8_t*)&tmp_header, PACKET_HEAD_SIZE, data_.begin());
}

uint16_t Packet::append_body (const std::string& data) {
    std::copy(data.begin(), data.end(), std::back_inserter(data_));

    packet_header header;
    memcpy(&header, (void*)&data_, PACKET_HEAD_SIZE);
    header.packet_len = data_.size() - PACKET_HEAD_SIZE; // update body size

    // update header
    for (uint32_t i = 0; i < PACKET_HEAD_SIZE; ++i) {
        data_[i] = *(uint8_t*)(&header + i);
    }
}

template<>
PMessagePtr Packet::GetMessage<PMessage>() {
    if (get_binary_protocol() != kProtobufBinaryProtocol) {
        MUX_ERROR("GetMessage(protobuf) type failed:{0}", get_binary_protocol());
        return nullptr;
    }

    PMessagePtr message = std::make_shared<PMessage>();
    if (!message->ParseFromArray((data_.data() + PACKET_HEAD_SIZE), data_.size() - PACKET_HEAD_SIZE))
    {
        MUX_ERROR("Message ParseFromString from string failed!");
        return nullptr;
    }
    return message;
}

}
