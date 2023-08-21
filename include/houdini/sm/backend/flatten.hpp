#pragma once

#include <boost/mp11/bind.hpp>
#include <boost/mp11/function.hpp>
#include <boost/mp11/list.hpp>
#include "houdini/sm/backend/transition.hpp"
#include "houdini/sm/backend/resolve_state.hpp"
#include "houdini/sm/backend/traits.hpp"
#include "houdini/sm/backend/remove_duplicates.hpp"
#include "houdini/sm/backend/type_list.hpp"

#include "houdini/util/mp11.hpp"
#include "houdini/util/mp_flatten.hpp"

namespace houdini {
namespace sm {

	
template <class State, class Parents> constexpr auto flattenSubTransitionTable(State state, Parents parents);

namespace detail {
//Generate list of transitions by appending the current transitions
//to a list of transitions of the substates. 
template <class Parents, class Transition> 
using FlattenSubTransitionTable = mp::mp_push_front<
	decltype(flattenSubTransitionTable(resolveSubStateParent(Transition{}), Parents{})), 
	detail::makeExtendedTransition<Parents, Transition>>;
	//this fails for multilevel substates. 

} //namespace detail


//how to do this?
//step 1: retrieve transition table
//step 2: get sub transition tables
//step 3: append transition tables to each other
template <class State, class ParentStates> 
constexpr auto flattenTransitionTableRecursive(State state, ParentStates){
	using TransitionTuple = decltype(makeTransitionTable(state));
	
	using BoundSubTransitionTable = mp::mp_bind_front<detail::FlattenSubTransitionTable, ParentStates>;
	using FlattenedTable = util::mp_flatten<mp::mp_transform_q<BoundSubTransitionTable, TransitionTuple>>;

	return FlattenedTable{};
}
/**
 * @brief Retrieves a list of all transitions in a state machine,
 * flattened into a single type list.
 * 
 * Recursively descends the state tree and retrieves the list
 * of transitions in each state. The ordering of the list is in the order 
 * the transitions are defined in `make_transition_table`, but bottom-up, where 
 * if a transition is directed to a sub-state machine, the transitions of the sub-SM 
 * (and by extension, any sub-SMs it may have) are listed first. 
 * 
 * @param state The state to retrieve transitions from. Normally the root state of the state machine
 * unless this function is being called recursively.
 */
template <class RootState> constexpr auto flattenTransitionTable(RootState root_state){
	using ParentStates = detail::TypeList<RootState>;

	return removeDuplicates(flattenTransitionTableRecursive(root_state, ParentStates{}));
}


template <class State, class Parents> 
constexpr auto flattenSubTransitionTable(State, Parents){
	if constexpr (mp::mp_similar<State, detail::TypeList<>>::value){
		// in this case, State is actually a type list of 
		//parents derived from a direct, entry or history state
		//therefore we must grab the front state
		using BackState = mp::mp_back<State>;
		if constexpr (HasTransitionTable<BackState>::value){
			using NewParents = mp::mp_push_front<Parents>;
			return flattenTransitionTableRecursive(BackState{}, NewParents{});
		}
		else {
			return detail::makeTypeList();
		}

	} else if constexpr (HasTransitionTable<State>::value){
		using NewParents = mp::mp_push_front<Parents, State>;
		return flattenTransitionTableRecursive(State{}, NewParents{});
	} else {
		return detail::makeTypeList();
	}
}


} // namespace sm
} // namespace houdini