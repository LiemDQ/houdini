#pragma once
#include "houdini/sm/backend/state.hpp"
#include "houdini/sm/backend/transition.hpp"
#include "houdini/sm/backend/event.hpp"

#include "houdini/util/types.hpp"

namespace houdini {
namespace sm {

/**
 * Intermediate transition objects for the state machine transition table.
 * Using operator overloading, the entries in each row of the transition table 
 * are used to construct Transition objects (internal or external) using 
 * `makeInternalTransition` or `makeTransition`. 
 * 
 * These transition objects are templated by the types that describe the transition.
 */

template <JEvent Event, class Guard, class Action> class TransitionEGA {
	public:
		constexpr TransitionEGA(const Guard& guard_, const Action& action_):
			guard(guard_),
			action(action_) 
			{}
		
		constexpr auto operator+() {
			return detail::makeInternalTransition<Event, Guard, Action>{};
		}
	
	public:
		const Guard guard;
		const Action action;

};

template <JEvent Event, class Guard> class TransitionEG {
	public:
		constexpr TransitionEG(const Guard& guard_):
			guard(guard_)
			{}
		
		template <class Action> constexpr auto operator/(const Action& action){
			return TransitionEGA<Event, Guard, Action> {guard, action};
		}

		constexpr auto operator+() {
			return detail::makeInternalTransition<Event, Guard, NoAction>{};
		}
	
	public:
		const Guard guard;
};

template <JEvent Event, class Action> class TransitionEA {
	public:
		constexpr TransitionEA(const Action& action_):
			action(action_)
			{}
		
		constexpr auto operator+(){
			return detail::makeInternalTransition<Event, NoGuard, Action>{};
		}
	
	public:
		const Action action;
};

/**
 * Full transition. Contains all the type information about the transition. 
 */
template <class Source, JEvent Event, class Guard, class Action> class TransitionSEGA {
	public:
		constexpr TransitionSEGA(const Guard& guard_, const Action& action_):
			guard(guard_),
			action(action_)
			{}
		
		template <class Target> constexpr auto operator=(const Target&){
			return detail::makeTransition<TState<Source>, Event, Guard, Action, Target>{};
		}
	
	private:
		const Guard guard;
		const Action action;
};

template <class Source, JEvent Event, class Guard> class TransitionSEG {
	public:
		constexpr TransitionSEG(const Guard& guard_):
			guard(guard_)
			{}
		
		template <class Target> constexpr auto operator=(const Target&){
			return detail::makeTransition<TState<Source>,Event, Guard, NoAction, Target>{};
		}
	
	private:
		const Guard guard;
};


template <class Source, JEvent Event, class Action> class TransitionSEA {
	public:
		constexpr TransitionSEA(const Action& action_):
			action(action_)
			{}
		
		template <class Target> constexpr auto operator=(const Target&){
			return detail::makeTransition<TState<Source>, Event, NoGuard, Action, Target>{};
		}
	
	private:
		const Action action;
};

template <class Source, JEvent Event> class TransitionSE {
	public:
		
		template <class Target> constexpr auto operator=(const Target&){
			return detail::makeTransition<TState<Source>, Event, NoGuard, NoAction, Target>{};
		}

};

template <class Source, class Action> class TransitionSA {
	public:
		constexpr TransitionSA(const Action& action_): action(action_) {}

		template <class Target> constexpr auto operator=(const Target&){
			return detail::makeTransition<TState<Source>, PLACEHOLDER_NO_EVENT_VALUE, NoGuard, Action, Target>{};
		}
	
	private:
		const Action action;
};

template <class Source, class Guard, class Action> class TransitionSGA {
	public:
		constexpr TransitionSGA(const Guard& guard_, const Action& action_):
		 guard(guard_),
		 action(action_)
		 {}
		
		template <class Target> constexpr auto operator=(const Target&) {
			return detail::makeTransition<TState<Source>,PLACEHOLDER_NO_EVENT_VALUE, Guard, Action, Target>{};
		}

	private:
		const Guard guard;
		const Action action;
};

template <class Source, class Guard> class TransitionSG {
	public:
		constexpr TransitionSG(const Guard& guard_):
		 guard(guard_)
		 {}
		
		template <class Target> constexpr auto operator=(const Target&){
			return detail::makeTransition<TState<Source>, PLACEHOLDER_NO_EVENT_VALUE, NoAction, Guard, Target>{};
		}
	
	private:
		const Guard guard;
};
	
} // namespace sm
} // namespace houdini