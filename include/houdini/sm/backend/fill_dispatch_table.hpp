#pragma once
#include <boost/mp11/list.hpp>
#include "houdini/sm/backend/algorithms.hpp"
#include "houdini/sm/backend/traits.hpp"
#include "houdini/sm/backend/index.hpp"
#include "houdini/sm/backend/dispatch_table.hpp"
#include "houdini/sm/backend/collect.hpp"
#include "houdini/sm/backend/collect_initial_states.hpp"
#include "houdini/sm/backend/flatten.hpp"
#include "houdini/sm/backend/flatten_internal_transition_table.hpp"
#include "houdini/sm/backend/resolve_state.hpp"
#include "houdini/sm/frontend/static_stack.hpp"

#include "houdini/util/mp11.hpp"
#include "houdini/util/type_name.hpp"
#include "houdini/util/static_typeid.hpp"
#include "houdini/util/types.hpp"
#include "houdini/util/enum_utils.hpp"

#include <iostream>
#include <cstddef>
#include <memory>
#include <string_view>
#include <type_traits>
#include <variant>
#include <functional>
#include <limits>
#include <array>
#include <cassert>


namespace houdini {
namespace sm {

namespace detail {
template <class Transition>
using TransitionHasDeferredEvents = HasDeferredEvents<decltype(ResolveExtendedInitialState<Transition>())>;


enum class HistoryType { //TODO: move this to an appropriate location
	NONE,
	SHALLOW,
	DEEP
};

/** @brief Determines the parents for a destination state, taking into account that 
 *	the destination could be further down in the hierarchy.
 *	
 *	If the state we're transitioning to has embedded states, we need to instead transition
 *	to its initial state. In that case, we need to get the index associated with the embedded initial state
 *	and add the state we're transitioning to its list of parents. 
 * 
 * @return A type list of the final destination state and its parents
 */
template <class Transition>
constexpr auto resolveInitialStateParents(Transition transition){
	auto target = transition.target();
	using TargetParents  = decltype(resolveDstParents(transition));
	if constexpr (HasTransitionTable<decltype(resolvhoudini(target))>::value 
	&& !is_history_state(target)){
		//history states should just specify the parent state instead of finding the 
		//initial nested state. The final destination state will be resolved at runtime
		return mp::mp_push_front<TargetParents, decltype(resolveDst(transition)), decltype(resolvhoudini(target))>{};
	} else {
		if constexpr (is_history_state(target)){
			//for a history state, there can be multiple nested parents. We want to add all of them
			//to the list. 
			return mp::mp_append<decltype(getParentStateList(target)), TargetParents>{};
		} else {
			//base case: just remove any wrapper pseudostates from the destination type.
			return mp::mp_push_front<TargetParents, decltype(resolvhoudini(target))>{};
		}
	}
}
} // namespace

constexpr auto getDeferringTransitions = [](auto root_state){
	using Transitions = decltype(flattenTransitionTable(root_state));
	return mp::mp_filter<detail::TransitionHasDeferredEvents, Transitions>{};
};

template <
	class SM,
	class TransitionTuple,
	class DispatchMap,
	class Dependencies>
constexpr void addDispatchTableEntry(
	SM&,
	TransitionTuple transition,
	DispatchMap& dispatch_map,
	JEvent event_id,
	Dependencies optional_dependency){

	constexpr std::size_t max_depth = SM::SM_DEPTH;

	using StateMap = typename SM::StateMap;

	//if the state we're transitioning to has embedded states, we need to instead transition
	//to its initial state. In that case, we need to get the index associated with the embedded initial state
	//and add the state we're transitioning to its list of parents.  
	//std::cout << "Transition: " << util::type_name(transition) << std::endl;
	auto dest_parents = detail::resolveInitialStateParents(transition);
	
	constexpr std::size_t from_index = getCombinedStateIndex(StateMap{}, resolveSrcParents(transition), resolveSrc(transition));
	constexpr std::size_t n_states = SM::NUM_STATES;

	StaticStack<StateIndex, max_depth> state_indices{
		get_state_indices<decltype(dest_parents), StateMap, n_states, max_depth>()};

	static_assert(from_index < n_states, "From index not found in dispatch map!");

	auto& dispatch_table = dispatch_map[event_id];
	auto dispatch_table_entry = makeDispatchEntry(
		transition,
		transition.action(),
		transition.guard(),
		optional_dependency);


	const bool is_history = resolveHistory(transition);
	const bool internal = transition.internal();
	const bool defer = false;
	const bool valid = true;

	dispatch_table[from_index].push_back(
		NextState<max_depth>{ 
			state_indices, 
			is_history, //TODO: change to enum ?
			defer, 
			valid, 
			internal, 
			std::move(dispatch_table_entry)
			}
		);
	//std::cout << "Final size: " << dispatch_table[from_index].back().destination_states.front() << std::endl;
}

/** @brief Add dispatch table entry for substates. This is required so that higher-level transitions
 * override transitions on states at a lower point in the hierarchy. If a state has substate,
 * iterate through its child states and add the transitions for the state into their 
 *
 */
template <
	class SM,
	class TransitionTuple,
	class DispatchMap,
	class Dependencies>
constexpr void addDispatchTableEntryForSubStates(
	SM&,
	TransitionTuple transition,
	DispatchMap& dispatch_map,
	JEvent event_id,
	Dependencies optional_dependency){
		
	if constexpr (transition.internal()){
		return;
	}

	constexpr auto parent_state = transition.source();
	using SrcParentList = decltype(resolveSrcParents(transition));
	// using ParentList = mp::mp_push_front<SrcParentList,std::decay_t<decltype(parent_state)>>;

	if constexpr (HasTransitionTable<decltype(parent_state)>::value){
		using ChildStates = decltype(collectListOfChildStates(parent_state));
		
		using StateMap = typename SM::StateMap;
		StateMap state_map;
		constexpr std::size_t max_depth = SM::SM_DEPTH;
		constexpr std::size_t n_states = SM::NUM_STATES;

		auto dest_parents = detail::resolveInitialStateParents(transition);
		//std::cout << "Type name of parents of substate: " << util::type_name(dest_parents) << std::endl;
		StaticStack<StateIndex, max_depth> state_indices = get_state_indices<decltype(dest_parents), StateMap, n_states, max_depth>();
		
		constexpr auto history = resolveHistory(transition);	
		constexpr auto defer = false;
		constexpr auto valid = true;
		
		mp::mp_for_each<ChildStates>( [&](auto state_list){
			//this will not work for more than 1 layer deep. Need to think about how to generalize this 
			//to multiple levels. 
			using Sourchoudini = mp::mp_front<decltype(state_list)>;
			using SourceParentList = mp::mp_append<mp::mp_rest<decltype(state_list)>, SrcParentList>;

			constexpr auto from_index = getCombinedStateIndex(state_map, SourceParentList{}, Sourchoudini{});
			//using a static assert here results in an internal compiler error with gcc; clang works fine
			assert(from_index < n_states && "From index not found in dispatch map!"); 
			// std::cout << "Substate State index size: " << state_indices.size() << "\n";
			// std::cout << "State index first entry: " << state_indices.back() << "\n";
			// std::cout << "Substate From index and event: " << from_index << ", " << event_id << std::endl; 

			auto& dispatch_table = dispatch_map[event_id];
			auto dispatch_table_entry = makeDispatchEntry(
					transition,
					transition.action(),
					transition.guard(),
					optional_dependency);
			
			const bool internal = transition.internal();

			dispatch_table[from_index].push_back(
				NextState<max_depth>{
					std::move(state_indices), 
					history, 
					defer, 
					valid, 
					internal,
					std::move(dispatch_table_entry)});
		
			}
		);
	} 
	//disable compiler warnings for unused parameters
	(void) event_id;
	(void) optional_dependency;
	(void) dispatch_map;
}


/**
 * @brief Fills the static_map of transitions with NextState entries containing 
 * information about the transition. 
 * 
 * 
 */
template <
	class SM,
	class DispatchMap,
	class Dependencies,
	class TransitionTuple>
constexpr auto fillDispatchTableWithTransitions(
	SM& sm,
	DispatchMap& dispatch_map,
	Dependencies&& optional_dependency,
	TransitionTuple) {
	
	using EventEnum = typename SM::Events;

	//constexpr auto event_ids = collectEventTypeIDsRecursiveFromTransitions(transitions);
	mp::mp_for_each<TransitionTuple>(
		[&sm, &optional_dependency, &dispatch_map](auto transition){

			JEvent event_id = transition.event();
			if (event_id == PLACEHOLDER_NO_EVENT_VALUE) {
				//set anonymous transition event id to last enum value + 1
				//to avoid going out of bounds on event_id static map array
				event_id = util::enum_max_value<EventEnum>() + 1;
			}
			addDispatchTableEntryForSubStates(
				sm,
				transition,
				dispatch_map,
				event_id,
				optional_dependency
			);
			addDispatchTableEntry(
				sm,
				transition,
				dispatch_map,
				event_id,
				optional_dependency
			);
		}
	);
}

template <class SM, class DispatchMap, class OptionalDependency>
constexpr auto fillDispatchTableWithDeferredEvents(
	SM& state_machine, DispatchMap& dispatch_map, OptionalDependency optional_dependency) {
	using DeferredTransitions = decltype(getDeferringTransitions(state_machine.root_state));
	constexpr std::size_t max_depth = SM::SM_DEPTH;
	using StateMap = typename SM::StateMap;
	StateMap state_map;
	constexpr std::size_t n_states = SM::NUM_STATES;

	mp::mp_for_each<DeferredTransitions>([&](auto transition){
		auto deferred_events = get_defer_events(resolveExtendedInitialState(transition));
		for (auto event_enum:deferred_events){
			JEvent event = static_cast<JEvent>(event_enum);
			if (event == transition.event()){
				
				const StateIndex from_index = getCombinedStateIndex(
					state_map, resolveSrcParents(transition), resolveSrc(transition));
				
				auto dest_parents = detail::resolveInitialStateParents(transition);
				
				assert(from_index < n_states && "From index not found in dispatch map!");
				//TODO: FIX THIS
				
				StaticStack<StateIndex, max_depth> state_indices = get_state_indices<decltype(dest_parents), StateMap, n_states, max_depth>();
				
				const bool history = resolveHistory(transition);
				const bool defer = true;
				const bool valid = true;
				
				auto& dispatch_table = dispatch_map[event];
				auto dispatch_table_entry = makeDispatchEntry(
						transition,
						transition.action(),
						transition.guard(),
						optional_dependency);

				bool internal = transition.internal();
				dispatch_table[from_index].push_back({
					NextState<max_depth>{
						std::move(state_indices),
						history,
						defer,
						valid,
						internal,
						std::move( dispatch_table_entry)
					}
				});
				break;
			}
		}
	});
}


template <class SM, class OptionalDependency>
constexpr auto fillDispatchTableWithExternalTransitions(
	SM& sm,
	OptionalDependency&& optional_dependency){
	
	fillDispatchTableWithTransitions(
		sm,
		sm.dispatch_map,
		optional_dependency,
		sortTransitionTableByParentSize(flattenTransitionTable(sm.root_state)) //needs to be flattened
	);

}


template <class SM, class OptionalDependency>
constexpr auto fillDispatchTableWithInternalTransitions(
	SM& sm,
	OptionalDependency&& optional_dependency){
	
	fillDispatchTableWithTransitions(
		sm,
		sm.dispatch_map,
		optional_dependency,
		sortTransitionTableByParentSize(flattenInternalTransitionTable(sm.root_state))
	);

}

} //namespace sm
} //namespace houdini
