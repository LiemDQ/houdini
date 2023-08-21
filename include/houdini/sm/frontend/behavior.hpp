#pragma once
#include "houdini/actor/context.hpp"
#include "houdini/brokers/message_broker.hpp"

#include <chrono>
#include <locale>

namespace houdini {
namespace sm {

class BaseBehavior {

	public:
		BaseBehavior() = default;
		BaseBehavior(int frequency) : update_frequency{frequency} {}
	
		virtual void onEntryImpl(act::BaseContext& context, brokers::BaseBroker& broker) = 0;
		virtual void onExitImpl(act::BaseContext& context, brokers::BaseBroker& broker) = 0;
		virtual void updateImpl(act::BaseContext& context, brokers::BaseBroker& broker) = 0;
		virtual ~BaseBehavior(){}
		std::chrono::milliseconds update_frequency{200};

	protected:

		std::chrono::time_point<std::chrono::steady_clock> last_update;
};

/**
 * \brief The behavior class represents a set of actions associated with a state. Subclasses of Behavior should defined entry, exit 
    and update methods as appropriate. Entry methods are called when the associated state is entered. Exit methods
	are called when the state is exited. Update methods are called at regular intervals while the state is active
	at a frequency determined by update_frequency. 
*/
template <typename Context, typename Broker>
class Behavior : public BaseBehavior {

	public:
		Behavior() = default;
		Behavior(int frequency) : BaseBehavior(frequency) {}
		
		virtual ~Behavior() {}
	
		void onEntryImpl(act::BaseContext& context, brokers::BaseBroker& broker) override final {
			onEntry(static_cast<Context&>(context), static_cast<Broker&>(broker));
		}

		void onExitImpl(act::BaseContext& context, brokers::BaseBroker& broker) override final {
			onExit(static_cast<Context&>(context), static_cast<Broker&>(broker));
		}

		void updateImpl(act::BaseContext& context, brokers::BaseBroker& broker) override final {
			if (this->update_frequency > std::chrono::milliseconds::zero() && std::chrono::steady_clock::now() - this->last_update >= this->update_frequency){
				update(static_cast<Context&>(context), static_cast<Broker&>(broker));
			}
		}
	private:
		virtual void onEntry(Context&, Broker&){}
		virtual void onExit(Context&, Broker&){}
		virtual void update(Context&, Broker&){}
};
}
}