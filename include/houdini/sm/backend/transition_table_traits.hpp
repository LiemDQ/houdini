#pragma once
#include "houdini/sm/backend/flatten.hpp"
#include "houdini/sm/backend/event.hpp"

#include "houdini/util/mp11.hpp"

#include <type_traits>

namespace houdini {
namespace sm {

constexpr auto is_anonymous_transition = [](auto transition) {return transition.event() == PLACEHOLDER_NO_EVENT_VALUE; };

constexpr auto is_history_transition = [](auto transition) { return is_history_state(transition.target());};

// template <class Transition>
// using IsAnonymousTransition = mp::mp_same<typename Transition::event_t, TEvent<NoEvent>>;

template <class Transition>
using IsHistoryTransition = IsHistoryState<typename Transition::target_t>;

template <class State> constexpr auto has_anonymous_transition(State root_state){
	using Transitions = decltype(flattenTransitionTable(root_state));
	bool ret = false;
	mp::mp_for_each<Transitions>(
		[&ret](auto transition){
			if (transition.event() == PLACEHOLDER_NO_EVENT_VALUE){
				ret = true;
			}
		}
	);
	return ret;
}

template <class State> constexpr auto has_history(State root_state){
	using Transitions = decltype(flattenTransitionTable(root_state));
	using HistoryTransitions = mp::mp_filter<IsHistoryTransition, Transitions>;

	return !mp::mp_empty<HistoryTransitions>::value;
}

template <class State> 
constexpr std::size_t history_size(State state, std::size_t size){
	if constexpr (has_history(state)){
		return size;
	}
	else return 0;
}

} //namespace sm
} //namespace houdini