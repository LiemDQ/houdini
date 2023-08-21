#pragma once
#include "houdini/sm/frontend/behavior.hpp"
#include "houdini/actor/context.hpp"
#include "houdini/brokers/message_broker.hpp"
#include "houdini/util/constants.hpp"

#include <cstddef>
#include <memory>
#include <array>
#include <chrono>
#include <ratio>

namespace houdini {
namespace sm {


template <typename Context = act::BaseContext, typename Broker = brokers::BaseBroker>
struct State {
    State() = default;
    State(int frequency) : update_frequency{frequency} {}

    virtual ~State() {}

    void onEntryImpl(Context& context, Broker& broker){
        this->onEntry(context, broker);
        for (auto& behavior:this->behaviors){
            if (behavior)
                behavior->onEntryImpl(context, broker);
            else break;
        }
    }

    void onExitImpl(Context& context, Broker& broker){
        this->onExit(context, broker);
        for (auto& behavior:this->behaviors){
            if (behavior)
                behavior->onExitImpl(context, broker);
            else break;
        }
    }

    void updateImpl(Context& context, Broker& broker){
        if (this->update_frequency > std::chrono::milliseconds::zero() && std::chrono::steady_clock::now() - this->last_update >= this->update_frequency){
            this->last_update = std::chrono::steady_clock::now();
            this->update(context, broker);
        }
        for (auto& behavior:this->behaviors){
            if (behavior)
                behavior->updateImpl(context, broker);
            else break;
        }
    }

    protected:
	template <typename... Behaviors>
	auto addBehaviors(Behaviors... behaviors_){
        static_assert(sizeof...(Behaviors) <= JANUS_MAX_BEHAVIORS, "Cannot supply more behaviors than maximum.");
        //utility function to create behaviors in the array
        this->behaviors = {std::unique_ptr<Behavior<Context,Broker>>(new Behaviors(std::move(behaviors_))...)};
		
	}
    std::array<std::unique_ptr<Behavior<Context,Broker>>, JANUS_MAX_BEHAVIORS> behaviors; 
    //this could be a vector but for some reason unique_ptrs won't get constructed properly when passed in via a parameter pack

    std::chrono::time_point<std::chrono::steady_clock> last_update;

    private:
	virtual void onEntry(Context&, Broker&){}
	virtual void onExit(Context&, Broker&){}
    virtual void update(Context&, Broker&){}

    std::chrono::milliseconds update_frequency{0};

    
};

} //namespace sm
} //namespace houdini