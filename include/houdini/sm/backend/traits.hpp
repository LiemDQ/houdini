#pragma once 
#include <boost/mp11/list.hpp>
#include "houdini/sm/backend/pseudo_states.hpp"

#include "houdini/util/mp11.hpp"

#include <functional>
#include <memory>
#include <utility>
#include <type_traits>

namespace houdini {
namespace sm {
namespace detail {

template <class State>
using BarhoudiniHasInternalTransitionTableImpl = decltype(std::declval<State>().make_internal_transition_table());

template <class State>
using HasInternalTransitionTable = mp::mp_valid<BarhoudiniHasInternalTransitionTableImpl, State>;
} //namespace detail

constexpr auto getParentState = [](auto state){
	return decltype(std::declval<typename decltype(state)::type>().getParentState())();
};

constexpr auto getBackState = [](auto state){
	return mp::mp_back<typename decltype(state)::type::parent>{};
};

constexpr auto getParentStateList = [](auto state){
	return decltype(std::declval<typename decltype(state)::type>().getParentStateList())();
};

constexpr auto getFullStateList = [](auto state){
	using ParentStates = decltype(std::declval<typename decltype(state)::type>().getParentStateList());
	using State = decltype(std::declval<typename decltype(state)::type>().getState());
	return mp::mp_push_front<ParentStates, State>{};
};

template <class State>
using GetParentState = typename State::type::parent;

constexpr auto getState = [](auto state){
	return decltype(std::declval<typename decltype(state)::type>().getState())();
};

template <class State>
using GetState = typename State::type::state;

constexpr auto makeInternalTransitionTable 
	= [](auto state){ return decltype(state)::type::make_internal_transition_table(); };

constexpr auto makeTransitionTable 
	= [](auto state){ return decltype(state)::type::make_transition_table(); };


namespace detail {

template <class StateID>
using HasInternalTransitionTableImpl = decltype(std::declval<typename StateID::type>().make_internal_transition_table());

template <class StateID>
using HasTransitionTableImpl = decltype(std::declval<typename StateID::type>().make_transition_table());

template <class StateID>
using HasEntryActionImpl = decltype(std::declval<typename StateID::type>().on_entry());

template <class StateID>
using HasExitActionImpl = decltype(std::declval<typename StateID::type>().on_exit());

template <class StateID>
using HasDeferredEventsImpl = decltype(std::declval<typename StateID::type>().defer_events());
}

template <class StateID>
using HasInternalTransitionTable = mp::mp_valid<detail::HasInternalTransitionTableImpl, StateID>;

template <class StateID>
using HasTransitionTable = mp::mp_valid<detail::HasTransitionTableImpl, StateID>;

template <class StateID>
using HasExitAction = mp::mp_valid<detail::HasExitActionImpl, StateID>;

template <class StateID>
using HasEntryAction = mp::mp_valid<detail::HasEntryActionImpl, StateID>;

template <class StateID>
using HasDeferredEvents = mp::mp_valid<detail::HasDeferredEventsImpl, StateID>;

namespace detail {
template <class Bashoudini, class State>
using IsBaseOfState = std::is_base_of<Bashoudini, typename State::type>;
}

template <typename State>
using IsEntryState = detail::IsBaseOfState<EntryPseudoState, State>;

template <typename State>
using IsExitState = detail::IsBaseOfState<ExitPseudoState, State>;

template <typename State>
using IsDirectState = detail::IsBaseOfState<DirectPseudoState, State>;

template <typename State>
using IsHistoryState = detail::IsBaseOfState<HistoryPseudoState, State>;

template <typename State>
using IsInitialState = detail::IsBaseOfState<InitialPseudoState, State>;

constexpr auto is_entry_state = [](auto state_type_id){
	return std::is_base_of_v<EntryPseudoState, typename decltype(state_type_id)::type>;
};


constexpr auto is_exit_state = [](auto state_type_id){
	return std::is_base_of_v<ExitPseudoState, typename decltype(state_type_id)::type>;
};


constexpr auto is_direct_state = [](auto state_type_id){
	return std::is_base_of_v<DirectPseudoState, typename decltype(state_type_id)::type>;
};

constexpr auto is_history_state = [](auto state_type_id){
	return std::is_base_of_v<HistoryPseudoState, typename decltype(state_type_id)::type>;
};

constexpr auto is_initial_state = [](auto state_type_id){
	return std::is_base_of_v<InitialPseudoState, typename decltype(state_type_id)::type>;
};


template <class T>
using IsNoAction = mp::mp_same<NoAction, T>;

template <class T>
using IsNoGuard = mp::mp_same<NoGuard, T>;

template <class T>
using IsAction = mp::mp_not<mp::mp_same<NoAction, T>>;

template <class T>
using IsGuard = mp::mp_not<mp::mp_same<NoGuard,T>>;


constexpr auto is_no_action = [](auto action){ 
	return std::is_same_v<decltype(action), NoAction>;
};

template <class Action> constexpr decltype(auto) is_action() {
	return !std::is_same_v<Action, NoAction>;
}

constexpr auto is_no_guard = [](auto guard){
	return std::is_same_v<decltype(guard), NoGuard>;
};

template <class Guard> constexpr decltype(auto) is_guard() {
	return !std::is_same_v<Guard, NoGuard>;
}


constexpr auto get_defer_events
	= [](auto state_type_id){ return decltype(state_type_id)::defer_events();};

} //namespace sm
} //namespace houdini