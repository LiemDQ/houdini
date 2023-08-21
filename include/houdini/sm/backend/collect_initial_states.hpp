#pragma once
#include "houdini/sm/backend/collect.hpp"
#include "houdini/sm/backend/algorithms.hpp"
#include "houdini/sm/backend/index.hpp"
#include "houdini/sm/backend/traits.hpp"


#include <houdini/util/mp11.hpp>

#include <vector>
#include <functional>
#include <utility>

namespace houdini {
namespace sm {


/**
 * @brief Collects the initial states for the parent states.
 */
template <class State> constexpr auto collectInitialStates(State parent_state){
	using ChildStates = decltype(collectChildStates(parent_state));
    using InitialStates = mp::mp_filter<IsInitialState,ChildStates>;
	using CollectedStates = mp::mp_transform<GetState,InitialStates>;

	return CollectedStates{};
}

namespace detail {
template <class ParentState>
using GetNumInitialStates = mp::mp_size<decltype(collectInitialStates(ParentState{}))>;
}

template <class Parents> constexpr auto initialStateSizes(Parents){
    using InitialSizes = mp::mp_transform<detail::GetNumInitialStates, Parents>;
    return InitialSizes{};
}

template <class State> constexpr auto maxInitialStates(State root_state){
    return mp::mp_max_element<decltype(initialStateSizes(collectParentStates(root_state))), mp::mp_less>::value;
}

} //namespace sm
} //namespace houdini