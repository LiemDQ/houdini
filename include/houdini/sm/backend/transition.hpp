#pragma once
#include "houdini/sm/backend/type_list.hpp"
#include <boost/mp11/function.hpp>
#include "houdini/util/types.hpp"
#include "houdini/util/mp11.hpp"

#include <type_traits>

namespace houdini {
namespace sm {
namespace detail {

/**
 * @brief Transition class. Encodes information about a transition. 
 * This is the final result of each entry (line) in a transition table.
 * Its template arguments must be default constructible. 
 * 
 * @par Contains utility functions to obtain the types that make up the transition.
 * This is useful for things that require type deduction or introspecting certain properties 
 * about the type.
 */
template <class Source, JEvent Event, class Guard, class Action, class Target> struct Transition {

	using source_t = Source;
	using guard_t = Guard;
	using action_t = Action;
	using target_t = Target;
	
	constexpr Transition()
		{}
	
	[[nodiscard]] static constexpr auto source() -> Source {
		return Source {};
	}

	[[nodiscard]] static constexpr auto event() -> JEvent {
		return Event;
	}

	[[nodiscard]] static constexpr auto action() -> Action {
		return Action {};
	}

	[[nodiscard]] static constexpr auto guard() -> Guard {
		return Guard {};
	}

	[[nodiscard]] static constexpr auto target() -> Target {
		return Target {};
	}

};

/**
 * @brief Internal transition class. This is functionally the same as a Transition class, 
 * except that it does not specify a target state to transition to. Internal transitions are 
 * used to trigger discrete actions that can occur inside a state, but should otherwise 
 * not lead to a change of state. 
 */
template <JEvent Event, class Guard, class Action> struct InternalTransition {
	//using event_t = Event;
	using guard_t = Guard;
	using action_t = Action;
	
	constexpr InternalTransition()
		{}

	static constexpr auto event() -> JEvent {
		return Event;
	}

	static constexpr auto action() -> Action {
		return Action {};
	}

	static constexpr auto guard() -> Guard {
		return Guard {};
	}

};


/**
 * @brief Contains type information about the transition parent 
 * (i.e. the state machine that will be transitioning)
 * as well as whether the transition is an internal transition. 
 * This is sufficient generality for `ExtendedTransition` to be used
 * in all cases where the state machine backend requires type meta-information about
 * a transition. 
 */
template <class Parent, class Source, JEvent Event, class Guard, class Action, class Target, bool Internal>
struct ExtendedTransition {
	using parent_t = Parent; //type list of parents. Parents are listed from lowest parent in front to highest parent in back. 
	using source_t = Source;
	using guard_t = Guard;
	using action_t = Action;
	using target_t = Target;

	constexpr ExtendedTransition()
		{}
	
	[[nodiscard]] static constexpr auto parent() {
		if constexpr (mp::mp_similar<detail::TypeList<Parent>, Parent>::value){
			return mp::mp_front<Parent>{};
		} else {
			return Parent{};
		}
	}

	[[nodiscard]] static constexpr auto parent_list() -> Parent {
		return Parent {};
	}

	[[nodiscard]] static constexpr auto source() -> Source {
		return Source {};
	}

	[[nodiscard]] static constexpr auto event() -> JEvent {
		return Event;
	}

	[[nodiscard]] static constexpr auto action() -> Action {
		return Action {};
	}

	[[nodiscard]] static constexpr auto guard() -> Guard {
		return Guard {};
	}

	[[nodiscard]] static constexpr auto target() -> Target {
		return Target {};
	}

	[[nodiscard]] static constexpr auto internal() -> bool {
		return Internal;
	}

};

/**
 * Utility alias to construct a Transition object. 
 */
template <class Source, JEvent Event, class Guard, class Action, class Target>
using makeTransition = Transition<Source, Event, Guard, Action, Target>;

template <class Source, JEvent Event, class Guard, class Action, class Target>
using makeDispatchEntry = Transition<Source, Event, Guard, Action, Target>;

constexpr auto internal_transition = [](auto event, auto guard, auto action) { 
		return InternalTransition<
			event,
			std::decay_t<decltype(guard)>,
			std::decay_t<decltype(action)>>
			{}; };

template <JEvent Event, class Guard, class Action>
using makeInternalTransition = InternalTransition<Event, Guard, Action>;

template <class Source, class Internal>
using ConvertInternalTransitionToTransition = Transition<
													Source, 
													Internal::event(), 
													typename Internal::guard_t, 
													typename Internal::action_t, 
													Source>;

constexpr auto extended_transition = [](auto parent, auto transition){
	return ExtendedTransition<
	std::decay_t<decltype(parent)>,
	std::decay_t<decltype(transition.source())>,
	transition.event(),
	std::decay_t<decltype(transition.guard())>,
	std::decay_t<decltype(transition.action())>,
	std::decay_t<decltype(transition.target())>,
	false>{}; 
};

template <class Parent, class Transition>
using makeExtendedTransition = ExtendedTransition<
	Parent, 
	typename Transition::source_t, 
	Transition::event(),
	typename Transition::guard_t,
	typename Transition::action_t,
	typename Transition::target_t,
	false>;


constexpr auto extended_internal_transition = [](auto parent, auto transition){
	return ExtendedTransition<
	std::decay_t<decltype(parent)>,
	std::decay_t<decltype(transition.source())>,
	transition.event(),
	std::decay_t<decltype(transition.guard())>,
	std::decay_t<decltype(transition.action())>,
	std::decay_t<decltype(transition.target())>,
	true>{}; 
};

template <class Parent, class Transition>
using makeExtendedInternalTransition = ExtendedTransition<
	Parent, 
	typename Transition::source_t, 
	Transition::event(),
	typename Transition::guard_t,
	typename Transition::action_t,
	typename Transition::target_t,
	true>;

} // namespace detail
} // namespace sm
} // namespace houdini
