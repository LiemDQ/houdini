#pragma once
#include "houdini/actor/context.hpp"
#include "houdini/sm/sm.hpp"
#include "houdini/brokers/message_broker.hpp"
#include "houdini/util/types.hpp"
#include "houdini/util/enum_utils.hpp"
#include "houdini/util/type_name.hpp"

#include <memory_resource>
#include <ratio>
#include <string_view>
#include <mutex>
#include <utility>
#include <cstddef>
#include <thread>
#include <chrono>
#include <cassert>
#include <atomic>
#include <type_traits>

namespace houdini {
namespace act {

template <
        class Events, 
        class RootState,        
        class Context, 
        class MessageBroker
        >
class BaseActor {
    public:
        BaseActor(const BaseActor&) = delete;
        BaseActor(BaseActor&&) = delete;

        template <typename... BrokerArgs>
        explicit BaseActor(Context context, BrokerArgs&... args): //create a copy of the context to ensure encapsulation
        execution_context(context),
        message_broker{util::type_name<RootState>(), &cv, args...}
        {}

        using StateMachine = SM<RootState,Events,Context,MessageBroker>;
    
    protected:
        
        SMResult processEvent(Events event){
            assert(util::enum_value_valid(event));
            return this->actor_sm.processEvent(event);
        }

        void processEvent(JEvent event_id){
            Events event = static_cast<Events>(event_id);
            return this->processEvent(event);
        }

        Context execution_context{}; 
        MessageBroker message_broker;
        StateMachine actor_sm = StateMachine(execution_context, message_broker);
        std::mutex context_mutex{};
        std::condition_variable cv{};
};


/**
 * @brief Actor object. Forms the core of the framework. Actors encapulate a state machine 
 * and an execution context, and interact with the outside world using a message
 * passing service. 
 * 
 */
template <
        class Events, 
        class RootState,        
        class Context, 
        class MessageBroker
        >
class Actor : public BaseActor<Events, RootState, Context, MessageBroker> {
    static_assert(std::is_base_of_v<BaseContext, Context>, "Context must be derived from BaseContext");
    static_assert(std::is_base_of_v<brokers::BaseBroker, MessageBroker>, "Message broker must be derived from BaseBroker");
    
    public:
        Actor(const Actor&) = delete;
        Actor(Actor&&) = delete;

        template <typename... BrokerArgs>
        explicit Actor(
            Context context,
            std::chrono::milliseconds update_freq = std::chrono::milliseconds(50), 
            std::pmr::memory_resource* resource_ptr = std::pmr::new_delete_resource(),
            BrokerArgs&... args): 
            update_time(update_freq),
            mem_resource_ptr(resource_ptr),
            BaseActor<Events, RootState, Context, MessageBroker>(context, args...){
            
            this->execution_context.actor_status = ActorStatus::IDLE;
        }

        template <typename... BrokerArgs>
        explicit Actor(BrokerArgs&... args)
        : Actor(Context(), args...)
        {}

        template <typename... BrokerArgs>
        explicit Actor(std::pmr::memory_resource* resource_ptr, BrokerArgs&... args)
        : Actor(Context(), std::chrono::milliseconds(50), resource_ptr, args...) 
        {}

        template <typename... BrokerArgs>
        explicit Actor(
            std::chrono::milliseconds update_freq, 
            std::pmr::memory_resource* resource_ptr = std::pmr::new_delete_resource(),
            BrokerArgs&... args): 
            Actor(Context(), update_freq, resource_ptr, args...)
            {}
        

        void run(){
            //TODO: need to actually change the ActorStatus based on 
            //what happens inside the state machine.

            //need to specify loop time
            
            auto looper_func = [this](auto&& func){
                this->current_time = std::chrono::steady_clock::now();
                while (this->execution_context.actor_status != ActorStatus::STOP){
                    auto next_loop_time = this->current_time + this->update_time;
                    func();
                    std::this_thread::sleep_until(next_loop_time);
                    this->current_time = next_loop_time;
                }
            };

            //TODO: eventually, consider replacing std::thread with platform-specific 
            //thread handles for granular configuration, in particular for setting thread priorities.

            broker_thread = std::thread(looper_func, [this](){this->brokerCallbackOnce();});
            update_thread = std::thread(looper_func, [this](){this->updateCallback();});

            //TODO: revise threading strategy. Currently, by splitting the main loops into
            //3 threads we are basically letting the kernel decide which thread to run,
            //and internal actions are scheduled "cooperatively". This may be sufficient if
            //we switch to using a deterministic executor for the message broker, 
            //but it remains to be seen. 

            auto unique_lock = std::unique_lock(this->context_mutex, std::defer_lock);
            while (this->execution_context.actor_status != ActorStatus::STOP){
                unique_lock.lock();
                this->cv.wait(unique_lock, [this](){return this->message_broker.hasEvents();});
                Events event = this->message_broker.getFirstEvent();
                [[maybe_unused]] SMResult result = this->processEvent(event);
                if (this->execution_context.stop_flag){
                    this->execution_context.actor_status = ActorStatus::STOP;
                }
                unique_lock.unlock();
            }
            
            update_thread.join();
            broker_thread.join();        
        }

        void runContinuous(){
            
        }

        ActorStatus status() const {
            return this->execution_context.status;
        }

    private:

        void brokerCallbackOnce(){
            //this is a crude and likely unnecessary lock, but ensures no race conditions 
            //in the system. 
            auto lock = std::lock_guard(this->context_mutex);
            this->message_broker.loopOnce();
        }

        void updateCallback(){
            auto lock = std::lock_guard(this->context_mutex);
            this->actor_sm.update();
        }
        
        template <typename Func>
        void loopOnce(Func f){
            this->current_time = std::chrono::steady_clock::now();
            f();
            std::this_thread::sleep_until(this->current_time + this->update_time);
        }

        std::pmr::memory_resource* mem_resource_ptr = std::pmr::new_delete_resource();
        JAllocator<std::byte> alloc = JAllocator<std::byte>{mem_resource_ptr};
        std::chrono::time_point<std::chrono::steady_clock> current_time;
        const std::chrono::milliseconds update_time = std::chrono::milliseconds(50);
        
        std::thread update_thread;
        std::thread broker_thread;    
};

} //namespace act
} //namespace houdini