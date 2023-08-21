#pragma once
#include "houdini/sm/backend/traits.hpp"

namespace houdini {
namespace sm {
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
	} else if constexpr (is_entry_state(dest)){
		return getState(dest);
	} else if constexpr (is_direct_state(dest)){
		return getState(dest);
	} else if constexpr (is_history_state(dest)){
		return mp::mp_front<decltype(collectInitialStates(getParentState(dest)))>{};
	} else if constexpr (is_initial_state(dest)){
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
		using TransitionParents = decltype(transition.parent_list());
		using StateParents = decltype(getParentStateList(src));
		return mp::mp_append<StateParents, TransitionParents>{}; 

	} else {
		return transition.parent_list();
	}
}


template <class Transition> constexpr auto resolveEntryAction(Transition transition){
	if constexpr (transition.internal()){
		return [](auto&&...) {};
	} else if constexpr (HasEntryAction<decltype(transition.target())>::value){
		return get_entry_action(transition.target());
	} else {
		return [](auto&&...) {};
	}
}

template <class Transition> constexpr auto resolveInitialStateEntryAction(Transition transition){
	if constexpr (has_substate_initial_state_entry_action(transition.target())) {
		return get_entry_action(mp::mp_at_c<decltype(collectInitialStates(transition.target())), 0>{});
	} else {
		return [](auto&&...){};
	}
}

template <class Transition> constexpr auto resolveExitAction(Transition transition){
	const auto hasPseudoExitAction = has_pseudo_exit_action(transition);
	(void)hasPseudoExitAction;

	if constexpr (transition.internal()){
		return [](auto&&...){};
	} else if constexpr (HasExitAction<decltype(transition.source())>::value){
		return get_exit_action(transition.source());
	} else if constexpr (hasPseudoExitAction){
		return get_exit_action(resolveSrcParent(transition));
	} else {
		return [](auto&&...){};
	}
}

template <class Transition> constexpr auto resolveNoAction(Transition transition){
	const auto isNoAction = is_no_action(transition.action());

	if constexpr (isNoAction){
		return [](auto&&...){};
	} else {
		return transition.action();
	}
}

/**
 * @brief Most general transition action wrapper. This takes the exit action of the source state,
 * the transition action, and entry action of the target state and the entry action of the entry substate
 * and wraps it into a single lambda so it can be passed as a single object to the dispatch table entry.
 * 
 * @param transition The transition to resolve the actions from.
 */
template <class Transition> constexpr auto resolveEntryExitAction(Transition transition){
	return [exitAction(resolveExitAction(transition)),
			action(resolveNoAction(transition)),
			entryAction(resolveEntryAction(transition)),
			initialStateEntryAction(resolveInitialStateEntryAction(transition))]
			(auto&&... params) {
				
				exitAction(std::forward<decltype(params)>(params)...);
				action(std::forward<decltype(params)>(params)...);
				entryAction(std::forward<decltype(params)>(params)...);
				initialStateEntryAction(std::forward<decltype(params)>(params)...);
			};
}

template <class Transition> constexpr auto resolveAction(Transition transition){
	const auto hasAction = has_action(transition);

	if constexpr (hasAction){
		return resolveEntryExitAction(transition);
	} else {
		return transition.action();
	}
}

template <class Transition> constexpr auto resolveHistory(Transition transition){
	if constexpr (is_history_state(transition.target())) {
		return true;
	} else {
		return false;
	}
}

}
}