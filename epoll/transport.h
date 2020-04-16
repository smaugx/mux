#pragma once

#include <string>
#include <memory>

namespace mux {

namespace transport {

class Transport {
public:
    Transport()                                  = default;
    Transport(const Transport& other)            = delete;
    Transport& operator=(const Transport& other) = delete;
    Transport(Transport&& other)                 = delete;
    Transport& operator=(Transport&& other)      = delete;
    virtual ~Transport()                         = default;

public:
    virtual int Start(const std::string& local_ip, uint16_t local_port) = 0;
    virtual int Stop() = 0;

};

typedef std::shared_ptr<Transport> TransportPtr;

} // end namespace transport

} // end namespace mux
