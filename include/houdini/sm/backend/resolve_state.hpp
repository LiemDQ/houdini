#pragma once
#include "houdini/util/mp11.hpp"
#include "houdini/sm/backend/traits.hpp"
#include "houdini/sm/backend/type_list.hpp"
#include <type_traits>

namespace houdini {
namespace sm {

/** @brief Utility functions to resolve transition sources and targets.
 * When these states are wrapped in a specifier, like a history transition or direct transition,
 * additional work needs to be done to unwrap the specifier and determine which state to transition to
 * and whether any additional parent states need to be added. That logic is encapsulated in 
 * these functions. 
 */

/**
 * @brief Returns the target state of the transition, or if this is a state with pseudostate
 * qualifiers, returns the parent state of the target state.
 * 
 * @param transition The transition to extract the target state from.
 */
template <class Transition> constexpr auto resolveSubStateParent(Transition transition){
	auto constexpr target = transition.target();
	if constexpr (is_entry_state(target)){
		return getBackState(target);
	} else if constexpr (is_direct_state(target)) {
		return getBackState(target);
	} else if constexpr (is_history_state(target)){
		return getBackState(target);
	} else {
		return target;
	}
}

template <class Transition> constexpr auto resolveSubStateParent(){
	return resolveSubStateParent(Transition{});
}

/** \brief Unwraps the state type if it is wrapped a pseudostate modifier like InitialState
	HistoryState, etc.
*/
template <class State> constexpr auto resolvhoudini(State state){
	if constexpr (is_entry_state(state)){
		return getState(state);
	} else if constexpr (is_direct_state(state)) {
		return getState(state);
	} else if constexpr (is_history_state(state)){
		return getParentState(state);
	} else if constexpr (is_initial_state(state)){
		return getState(state);
	} else {
		return state;
	}
}

/** \brief Unwraps the state type if it is wrapped in a pseudostate modifier. For EntryStates and DirectStates
	resolve the parent furthest to the right (highest up on the hierarchy). This is done to ensure 
	no parent states are skipped. 
*/
template <class State> constexpr auto resolveBackState(State state){
	if constexpr (is_entry_state(state)){
		return getBackState(state);
	} else if constexpr (is_direct_state(state)) {
		return getBackState(state);
	} else if constexpr (is_history_state(state)){
		return getParentState(state);
	} else if constexpr (is_initial_state(state)){
		return getState(state);
	} else {
		return state;
	}
}

/**
 * @brief Returns the target state of the transition, stripping away any pseudostate 
 * qualifiers like a history state, direct state or entry state. 
 * 
 * @param transition The transition to extract the target state from.
 */
template <class Transition> constexpr auto resolveDst(Transition transition){
	auto constexpr dest = transition.target();

	if constexpr (HasTransitionTable<decltype(dest)>::value){
		using initial_substate = mp::mp_front<decltype(collectInitialStates(dest))>;
		return resolvhoudini(initial_substate{});
	} else if constexpr (is_direct_state(dest) || is_entry_state(dest)){
		if constexpr (HasTransitionTable<decltype(getState(dest))>::value){
			using initial_substate = mp::mp_front<decltype(collectInitialStates(getState(dest)))>;
			return resolvhoudini(initial_substate{});
		} else {
			return getState(dest);
		}
	} else if constexpr (is_history_state(dest)){
		return mp::mp_front<decltype(collectInitialStates(getParentState(dest)))>{};
	} else if constexpr (is_initial_state(dest)){
		static_assert(HasTransitionTable<decltype(getState(dest))>::value, "Nested initial states are not supported.");
		return getState(dest);
	} else {
		return dest;
	}
}

template <class Transition> constexpr auto resolveDstParent(Transition transition){
	auto constexpr target = transition.target();

	if constexpr (HasTransitionTable<decltype(target)>::value){
		return target;
	} else if constexpr (is_entry_state(target)){
		return getParentState(target);
	} else if constexpr (is_direct_state(target)){
		return getParentState(target);
	} else if constexpr (is_history_state(target)){
		return getParentState(target);
	} else {
		return transition.parent();
	}
}

template <class Transition> constexpr auto resolveDstParents(Transition transition){
	auto target = transition.target();
	if constexpr(is_entry_state(target) || is_direct_state(target)){
		using TransitionParents = decltype(transition.parent_list());
		using StateParents = decltype(getParentStateList(target));
		return mp::mp_append<StateParents, TransitionParents>{};

	} else if constexpr (is_history_state(target)){
		return transition.parent_list();
	} else {
		return transition.parent_list();
	}
}

template <class Transition> constexpr auto resolveSrc(Transition transition){
	auto constexpr src = transition.source();

	if constexpr (is_initial_state(src)){
		return getState(src);
	} else if constexpr (is_exit_state(src)){
		return getState(src);
	} else if constexpr (is_direct_state(src)){
		return getState(src);
	} else {
		return src;
	}
}

template <class Transition> constexpr auto resolveSrcParent(Transition transition) {
	auto constexpr src = transition.source();

	if constexpr (is_exit_state(src)) {
		return getParentState(src);
	} else if constexpr (is_direct_state(src)){
		return getParentState(src);
	} else {
		return transition.parent();
	}
}

template <class Transition> constexpr auto resolveSrcParents(Transition transition){
	auto src = transition.source();
	if constexpr (is_exit_state(src) || is_direct_state(src)){
		//in this case, the originating state is further down in the state hierarchy.
		//therefore we must append its parent states to the transition parent states 
		//(which only represent the parents at the current state level)
		using TransitionParents = decltype(transition.parent_list());
		using StateParents = decltype(getParentStateList(src));
		return mp::mp_append<StateParents, TransitionParents>{}; 

	} else {
		return transition.parent_list();
	}
}

template <class Transition> constexpr auto resolveHistory(Transition transition){
	if constexpr (is_history_state(transition.target())) {
		return true;
	} else {
		return false;
	}
}
} //namespace sm
} //namespace houdini