#pragma once
#include "houdini/util/types.hpp"

#include <mutex>
#include <string_view>
#include <string>
#include <condition_variable>

namespace houdini {
namespace brokers {

class BaseBroker {
    public:

    std::string_view name;
    std::condition_variable* cv; 

};

/**
 * Represents an abstraction for receiving messages over a message bus. 
 * Specializations should inherit from this class. 
 * 
 * Right now this class doesn't do anything but this may change in the future
 */
template <typename EventEnum>
class MessageBroker : public BaseBroker {
    public:
    MessageBroker(const std::string_view name_ = "none", std::condition_variable* cv_ = nullptr) : BaseBroker{name_, cv_} {}

    virtual ~MessageBroker() = default;

    MessageBroker(const MessageBroker&) = delete;

    void queueEvent(EventEnum event) {
        this->event_queue.push(event);
        if (this->event_queue.size() == 1){
            this->cv->notify_one();
        }
    }

    EventEnum getFirstEvent() {
        EventEnum event = this->event_queue.front();
        this->event_queue.pop();
        return event;
    }

    std::size_t getNumEvents() {
        return this->event_queue.size();
    }

    bool hasEvents() {
        return !this->event_queue.empty();
    }

    protected:
    JQueue<EventEnum> event_queue;
    virtual void loopOnce() {}
    virtual void loop() {}
};

} //namespace 
} //namespace houdini