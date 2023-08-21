#pragma once
#include <cstddef>
#include "houdini/sm/backend/traits.hpp"
#include "houdini/sm/backend/index.hpp"
#include "houdini/sm/frontend/static_stack.hpp"

#include "houdini/util/unpack_tuple.hpp"
#include "houdini/util/type_name.hpp"
#include "houdini/util/types.hpp"

#include <array>
#include <functional>
#include <memory>
#include <string_view>
#include <utility>
#include <type_traits>

namespace houdini {
namespace sm {

template <class T> auto get(std::reference_wrapper<T> ref) -> auto& {
	return ref.get();
}

template <class T> T& get(T& ref){
	return ref;
}

struct AbstractEventWrapper {
	virtual ~AbstractEventWrapper() = default;
};

template <class Event>
struct EventWrapper final : AbstractEventWrapper {
	Event ev;
	explicit EventWrapper(Event&& event) : ev(std::move(event)) {}
};

/**
 * @brief Abstract `DispatchTableEntry` base class. This class is needed 
 * so that the type information about the transition (aside from the event)
 * is not required to store the entries into a container struct. 
 */
struct IDispatchTableEntry {
	virtual ~IDispatchTableEntry() = default;
	virtual void executeAction(JEvent& event) = 0;
	virtual bool executeGuard(JEvent& event) = 0;
};

/**
 * @brief DispatchTableEntry is responsible for storing transition actions  
 * and guards so that they can be accessed when a specific transition is called. 
 * The "action" and "guard" types are functors or lambdas (in C++20, where
 * lambdas can be default constructible)
 */
template <
	bool Internal,
	class Action,
	class Guard,
	class OptionalDependency>
class DispatchTableEntry final: public IDispatchTableEntry {
	public:
		constexpr DispatchTableEntry(
			Action action_,
			Guard guard_,
			OptionalDependency optional_dependency_) :
			action(action_),
			guard(guard_),
			optional_dependency(optional_dependency_)
			{}
		
		void executeAction(JEvent& event) override {
			if constexpr(is_action<Action>()){
				[](	auto& action_,
					auto& event_,
					auto& optional_dependencies){
						util::unpack(
							[&action_, &event_](auto&... optional_dependency_){
								action_(event_, get(optional_dependency_)...);
							},
							optional_dependencies);
					}(this->action, event, this->optional_dependency);

			}
		}

		bool executeGuard(JEvent& event) override {
			if constexpr(is_guard<Guard>()) {
				return [](
					auto& guard_,
					auto& event_,
					auto& optional_dependencies) {
						return util::unpack(
							[&guard_, &event_](auto&... optional_dependency_){
								if constexpr (Internal){
									return guard_(event_, get(optional_dependency_)...);
								}
								else {
									return guard_(event_, get(optional_dependency_)...);
								}
							},
							optional_dependencies
						);
					}(this->guard, event, this->optional_dependency);
			} 
			else {
				return true;
			}
		}

	private:
		Action action;
		Guard guard;
		OptionalDependency optional_dependency;
};


template <
	class Transition,
	class Action,
	class Guard,
	class Dependency>
constexpr auto makeDispatchEntry(
	Transition transition,
	Action action,
	Guard guard,
	//JEvent event_type_id,
	Dependency optional_dependency){
	
	return std::unique_ptr<IDispatchTableEntry>( //TODO: check if this needs special memory allocation considerations
		new DispatchTableEntry<
			transition.internal(),
			Action,
			Guard,
			decltype(optional_dependency)>(action, guard, optional_dependency));
}

/**
 * @brief NextState contains the information needed to transition from one 
 * state to the next. 
 * 
 * @par The type-dependent information is hidden behind a pointer to 
 * an abstract interface to prevent the type information from bubbling up to the 
 * NextState struct, which would lead to excessive metaprogramming complexity and
 * compile times. 
 */
template <std::size_t Depth>
struct NextState {
	StaticStack<StateIndex, Depth> destination_states;
	bool history {}; //change to enum 
	bool defer {};
	bool valid = false;
	bool internal = false;
	std::unique_ptr<IDispatchTableEntry> transition = nullptr;
	//TODO: if we want state machines to be copyable 
	//(e.g. for replication/voting purposes) then we must define
	//a custom copy constructor for NextState , as unique_ptrs are not copyable
	//by default.
};

} //namespace sm
} //namespace houdini