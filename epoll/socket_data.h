#pragma once

#include <string>
#include <memory>

namespace mux {

namespace transport {

typedef struct SocketData {
public:
    SocketData()
        : fd_ { -1 },
          msg_ { "" } {}
    SocketData(int32_t fd, const std::string& msg)
        : fd_ { fd },
          msg_ { msg } {}

    int32_t fd_ { -1 };
    std::string msg_;
} SocketData;

typedef std::shared_ptr<SocketData> SocketDataPtr;

}

}
