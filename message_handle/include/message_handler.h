#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <memory>
#include <vector>
#include <thread>
#include <map>
#include <mutex>
#include <condition_variable>
#include <queue>

#include "mbase/include/packet.h"

namespace mux {

namespace transport {

static const uint32_t kMaxConsumer = 4;

class ThreadConsuemr;
class MessageHandler;

class ThreadConsumer {
public:
    ThreadConsumer(const ThreadConsumer& other)            = delete;
    ThreadConsumer& operator=(const ThreadConsumer& other) = delete;
    ThreadConsumer(ThreadConsumer&& other)                 = delete;
    ThreadConsumer& operator=(ThreadConsumer&& other)      = delete;

    ThreadConsumer(
            std::shared_ptr<MessageHandler> message_handler,
            std::mutex& mutex,
            std::condition_variable& cond_var);
    virtual ~ThreadConsumer();

public:
    void Join();
    void RegisterOnDispatchCallback(callback_recv_t callback);
    void UnRegisterOnDispatchCallback();

private:
    void LoopHandleMessage();

private:
    std::shared_ptr<MessageHandler> message_handler_;
    std::mutex& mutex_;
    std::condition_variable& cond_var_;
    bool destroy_ {false};
    std::shared_ptr<std::thread> loop_thread_ {nullptr};
    std::mutex callback_mutex_;
    callback_recv_t callback_;
};

typedef std::shared_ptr<ThreadConsumer> ThreadConsumerPtr;


class MessageHandler : public std::enable_shared_from_this<MessageHandler> {
//class MessageHandler {
public:
    MessageHandler(const MessageHandler&)                 = delete;
    MessageHandler& operator=(const MessageHandler&)      = delete;
    MessageHandler(MessageHandler&&)                      = delete;
    MessageHandler& operator=(MessageHandler&&)           = delete;

    MessageHandler();
    ~MessageHandler();

public:
    void HandleMessage(PacketPtr& msg);
    PacketPtr GetMessageFromQueue();
    void RegisterOnDispatchCallback(callback_recv_t callback);
    void UnRegisterOnDispatchCallback();
    void Init();
    void Join();

private:
    std::mutex priority_queue_map_mutex_;
    std::map<uint32_t, std::queue<PacketPtr>> priority_queue_map_;
    std::mutex mutex_;
    std::condition_variable cond_var_;
    std::vector<ThreadConsumerPtr> consumer_vec_;
};

}  // namespace transport

}  // namespace top

