#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace mux {

namespace transport {

class ThreadConsuemr;
class MultiMessageHandler;

class ThreadConsumer {
public:
    ThreadConsumer(const ThreadConsumer& other)            = delete;
    ThreadConsumer& operator=(const ThreadConsumer& other) = delete;
    ThreadConsumer(ThreadConsumer&& other)                 = delete;
    ThreadConsumer& operator=(ThreadConsumer&& other)      = delete;

    ThreadConsumer(
            std::shared_ptr<MultiMessageHandler> multi_handler,
            std::mutex& mutex,
            std::condition_variable& cond_var);
    virtual ~ThreadConsumer();

public:

};

}  // namespace transport

}  // namespace top

