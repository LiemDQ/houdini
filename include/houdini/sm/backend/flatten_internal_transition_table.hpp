#pragma once 

#include "houdini/sm/backend/collect.hpp"
#include "houdini/sm/backend/traits.hpp"
#include "houdini/sm/backend/transition.hpp"
#include "houdini/sm/backend/type_list.hpp"

#include "houdini/util/mp11.hpp"
#include "houdini/util/mp_flatten.hpp"

namespace houdini {
namespace sm {


namespace detail {
	template <typename L>
	using NotEmpty = mp::mp_not<mp::mp_empty<L>>;
}

/**
 * @brief Returns a tuple of internal transitions of a state if it exists.
 * Otherwise a empty tuple is returned. Source and target of the
 * transition are set to parentstate as a placeholder and need to
 * be filled with all child states of the particular state.
 *
 * @param parent_state State for which the internal transitions should be returned
 *
 */
constexpr auto getInternalTransitionTable = [](auto parent_state){
	using Parent = decltype(parent_state);
	using ParentList = detail::TypeList<Parent>;

	if constexpr (HasInternalTransitionTable<Parent>::value){
		using InternalTable = decltype(makeInternalTransitionTable(parent_state));

		using CreateInternalTransition = mp::mp_bind_front<detail::ConvertInternalTransitionToTransition, Parent>;
		using CreateExtendedTransition = mp::mp_bind_front<detail::makeExtendedTransition, ParentList>;

		using Transitions = mp::mp_transform_q<CreateInternalTransition,InternalTable>;
		using ExtendedTransitions = mp::mp_transform_q<CreateExtendedTransition,Transitions>;
		return ExtendedTransitions{};
		
	} else {
		return detail::makeTypeList();
	}
};

/**
 * @brief Extends an internal transitions to all provided states
 *
 * @param internal_transition Internal transition that should be extended
 * @param states tuple of states
 */
template <class Transition, class States>
constexpr auto extendInternalTransition(Transition, States){

	using GetInternalTransition = mp::mp_bind_back<detail::ConvertInternalTransitionToTransition, Transition>;
	using InternalTransitions = mp::mp_transform_q<GetInternalTransition,States>;
	
	using GetExtendedInternalTransition = mp::mp_bind_front<detail::makeExtendedInternalTransition,typename Transition::parent_t>;
	using Transformed = mp::mp_transform_q<GetExtendedInternalTransition,InternalTransitions>;

	return Transformed {};
}



namespace detail {

template <class Parent, class Transition> 
using ExtendInternalTransitionTableOfParent = mp::mp_if<
	HasInternalTransitionTable<Parent>,
	decltype(extendInternalTransition(Transition{}, collectChildStates(Parent{}))),
	decltype(detail::makeTypeList())>;

template <class Parent> 
using ExtendInternalTransitionTable = mp::mp_transform_q<
	mp::mp_bind_front<ExtendInternalTransitionTableOfParent, Parent>,
	decltype(getInternalTransitionTable(Parent{}))>;
}

/**
 * @brief Returns the internal transitions for each state
 * 
 * @param states a tuple of states
 * 
 * @return A default constructed type list of the internal transitions
 */

template <class States>
constexpr auto getInternalTransitions(States){
	using RawInternalTransitions = mp::mp_transform<detail::ExtendInternalTransitionTable,States>;
	using NonEmptyInternalTransitions = mp::mp_filter<detail::NotEmpty,RawInternalTransitions>;
	using FlattenedTransitions = util::mp_flatten<NonEmptyInternalTransitions>;

	return FlattenedTransitions{};
}

/**
 * @brief Retrieves the full list of 
 * 
 * @param root_state The root state of the state machine
 * 
 * @return A tuple of extended internal transitions reachable from a given root_state
 */
template <class State> constexpr auto flattenInternalTransitionTable(State root_state){
	using CollectedStates = decltype(collectStatesRecursive(root_state));
	using FlattenedInternalTable = util::mp_flatten<decltype(getInternalTransitions(CollectedStates{}))>;

	return FlattenedInternalTable {};
}

} //namespace sm
} //namespace houdini