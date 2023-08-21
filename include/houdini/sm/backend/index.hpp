#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "houdini/sm/backend/index_defs.hpp"
#include "houdini/sm/backend/collect.hpp"
#include "houdini/sm/backend/algorithms.hpp"

#include "houdini/util/static_typeid.hpp"

namespace houdini {
namespace sm {

template <class State> constexpr auto getCombinedStateTypeIDs(State root_state){
	using CombinedStates = decltype(collectStatesWithParents(root_state));

	return CombinedStates{};
}

template <class ParentState, class State> constexpr auto getCombinedStateTypeID(ParentState, State){
	using CombinedTypeID = mp::mp_push_front<ParentState, State>;
	return CombinedTypeID{};
}

template<class StateTypeIDs, class ParentState, class State> 
constexpr StateIndex getCombinedStateIndex(StateTypeIDs combined_state_type_ids, ParentState parent_state, State state){
	return index_of_v(combined_state_type_ids, getCombinedStateTypeID(parent_state, state));
}

constexpr auto getActionIndex = [](auto root_state, auto action){
	return index_of_v(collectActionsRecursive(root_state), action);
};

constexpr auto getGuardIndex = [](auto root_state, auto guard){
	return index_of_v(collectGuardsRecursive(root_state), guard);
};

} //namespace sm
} //namespace houdini