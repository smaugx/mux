#include "message_hande/message_hander.h"

#include "mbase/packet.h"
#include "mbase/mux_log.h"


namespace mux {

namespace transport {

ThreadConsumer::ThreadConsumer(
        std::shared_ptr<MessageHandler> message_handler,
        std::mutex& mutex,
        std::condition_variable& cond_var)
    : message_handler_ { message_handler },
      mutex_ {mutex},
      cond_var_ {cond_var_} {
    loop_thread_.reset(new std::thread(&ThreadConsumer::LoopHandleMessage, this));
}

ThreadConsumer::~ThreadConsumer() {
    Join();
    MUX_INFO("threadconsumer stop");
}


void ThreadConsumer::RegisterOnDispatchCallback(OnDispatchCallback callback) {
    assert(callback);
    std::unique_lock<std::mutex> lock(callback_mutex_);
    assert(!callback_);
    callback_ = callback;
    MUX_INFO("register dispatch callback in threadconsumer");
}


void ThreadConsumer::UnRegisterOnDispatchCallback() {
    std::unique_lock<std::mutex> lock(callback_mutex_);
    callback_ = nullptr;
    MUX_INFO("unregister dispatch callback in threadconsumer");
}

void ThreadConsumer::Join() {
    destroy_ = true;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        cond_var_.notify_all();
    }

    if (loop_thread_) {
        loop_thread_->join();
        loop_thread_.reset();
        loop_thread_ = nullptr;
    }
}

void ThreadConsumer::LoopHandleMessage() {
    while (!destroy_) {
        while (!destroy_) {
            PacketPtr packet = message_handler_->GetMessageFromQueue();
            if (!packet) {
                break;
            }

            if (!callback_) {
                MUX_WARN("thread consumer dispatch callback not registered");
                continue;
            }
            callback_(packet);
        }

        /*
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_var_.wait_for(lock, std::chrono::milliseconds(10));
        }
        */

        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 10 ms
    }
}

// end class ThreadConsumer

MessageHandler::MessageHandler()
    : priority_queue_map_mutex_(),
      priority_queue_map_(),
      mutex_(),
      cond_var_(),
      consumer_vec_() {
    for (uint32_t i = 0; i < kMaxConsumer; ++i) {
        consumer_vec_.push_back(std::make_shared<ThreadConsumer>(shared_from_this(), mutex_, cond_var_));
    }
    MUX_INFO("create {0} threadconsumer", kMaxConsumer);

    for (uint32_t i = 0; i < kMaxPacketPriority; ++i) {
        priority_queue_map_[i] = std::queue<PacketPtr>();
    }
    MUX_INFO("create {0} priority map_queue", kMaxPacketPriority);
}

MessageHandler::~MessageHandler() {
    Join();
    MUX_INFO("MessageHandler stop");
}

PacketPtr MessageHandler::GetMessageFromQueue() {
    std::unique_lock<std::mutex> lock(priority_queue_map_mutex_);
    for (uint32_t i = 0; i < kMaxPacketPriority; ++i) {
        if (!priority_queue_map_[i].empty()) {
            PacketPtr packet = priority_queue_map_[i].front();
            priority_map_queue_[i].pop();
            return packet;
        }
    }
    return nullptr;
}


}

}
