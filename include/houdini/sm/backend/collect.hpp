#pragma once

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/list.hpp>
#include "houdini/sm/backend/remove_duplicates.hpp"
#include "houdini/sm/backend/traits.hpp"
#include "houdini/sm/backend/flatten.hpp"
#include "houdini/sm/backend/resolve_state.hpp"
#include "houdini/sm/backend/type_list.hpp"

#include "houdini/util/mp11.hpp"
#include "houdini/util/mp_flatten.hpp"

namespace houdini {
namespace sm {

namespace detail { //utility functions
constexpr auto resolveInitialState = [](auto transition){
	if constexpr (is_initial_state(transition.source())){
		return getState(transition.source());
	} else {
		return transition.source();
	}
};

template <class Transition>
constexpr auto ResolveInitialState() {
	if constexpr (is_initial_state(Transition::source())){
		return getState(Transition::source());
	} else {
		return Transition::source();
	}
}

constexpr auto resolveExtendedInitialState = [](auto transition){
	if constexpr (is_initial_state(transition.source())){
		return getState(transition.source());
	} else {
		return transition.source();
	}
};  


template <class Transition>
constexpr auto ResolveExtendedInitialState(){
	if constexpr (is_initial_state(Transition::source())){
		return getState(Transition::source());
	} else {
		return Transition::source();
	}
}

// constexpr auto extractExtendedStates = [](auto transition){
// 	return detail::makeTypeList(resolveExtendedInitialState(transition), transition.target());
// };

template <class Transition>
using ExtractExtendedStates = mp::mp_list<decltype(ResolveExtendedInitialState<Transition>()), typename Transition::target_t>;

constexpr auto extractStates = [](auto transition){
	return detail::makeTypeList(transition.source(), transition.target());
};

template <class Transition> 
using ExtractStates = mp::mp_list<typename Transition::source_t,typename Transition::target_t>;

const auto collectAction = [](auto transition){return transition.action();};
const auto collectGuard = [](auto transition){return transition.guard();};

template <class Transition>
using CollectAction = typename Transition::action_t;

template <class Transition>
using CollectGuard = typename Transition::guard_t;

} //namespace 

// ACTIONS //

const auto collectActionsRecursive = [](auto state){
	using CollectedActions = mp::mp_transform<detail::CollectAction, decltype(flattenTransitionTable(state))>;
	return removeDuplicates(CollectedActions{});
};

// GUARDS //
const auto collectGuardsRecursive = [](auto state){
	using CollectedGuards = mp::mp_transform<detail::CollectGuard, decltype(flattenTransitionTable(state))>;
	return removeDuplicates(CollectedGuards{});
};


// STATES //


template <class State> constexpr auto collectChildStatesRecursive(State parent_state){
	return removeDuplicates(util::mp_flatten<mp::mp_transform<detail::ExtractExtendedStates, decltype(flattenTransitionTable(parent_state))>>{});
}

template <class State> constexpr auto collectStatesRecursive(State parent_state){
	return removeDuplicates(mp::mp_push_front<decltype(collectChildStatesRecursive(parent_state)), State>{});
}


namespace detail {

template <class Transition>
using ToParentState = mp::mp_front<typename Transition::parent_t>;

template <class Transition, class Source = typename Transition::source_t>
using GetSrcStateAndParents = mp::mp_push_front<typename Transition::parent_t,decltype(resolveBackState(Source{}))>;

template <class Transition, class Target = typename Transition::target_t>
using GetDstStateAndParents = mp::mp_push_front<typename Transition::parent_t, decltype(resolveBackState(Target{}))>;

template <class Transition>
using GetUnresolvedSrcStateAndParents = mp::mp_push_front<typename Transition::parent_t, typename Transition::source_t>;

} //namespace detail

template <class State> constexpr auto collectParentStates(State state){
	using Transitions = decltype(flattenTransitionTable(state));
	using ParentStates = mp::mp_transform<detail::ToParentState, Transitions>;

	return removeDuplicates(ParentStates{});
}

template <class RootState> 
constexpr auto collectStatesWithParents(RootState root_state){
	using Transitions = decltype(flattenTransitionTable(root_state));
	//from transition type list, get list of states, including their parents. 
	//for DirectStates and EntryStates, only collect the highest level state in their transition 
	//to avoid malformed parent state lists. This assumes that there is another way to reach the deepest child states, 
	//which in the vast majority of cases should be true.
	using SrcStatesWithParents = mp::mp_transform<detail::GetSrcStateAndParents, Transitions>;
	using DstStatesWithParents = mp::mp_transform<detail::GetDstStateAndParents, Transitions>;
	using StatesWithParents = mp::mp_append<SrcStatesWithParents,DstStatesWithParents>;
	using FullStatesWithParents = mp::mp_push_front<StatesWithParents, detail::TypeList<RootState>>;

	return removeDuplicates(FullStatesWithParents{});
}

template <class State> constexpr auto collectChildStates(State state){

	return removeDuplicates(
		util::mp_flatten<mp::mp_transform<detail::ExtractStates, decltype(makeTransitionTable(state))>>{}
	);
}

/**
 @brief Collects a list of lists of child states of the with their parents 
 */
template <class State> 
constexpr auto collectListOfChildStates(State parent_state){
	using StateList = decltype(collectStatesWithParents(parent_state));
	return mp::mp_rest<StateList>{};
}




} // namespace sm 
} // namespace houdini