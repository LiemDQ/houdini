#pragma once
#include "houdini/sm/backend/type_list.hpp"
#include "houdini/util/mp11.hpp"

namespace houdini {
namespace sm {

//Dummy type used to indicate that a transition has no action associated with it.
struct NoAction {};

//Dummy type used to indicate that a transition has no guard associated with it.
struct NoGuard {};

template <class Source> struct TState;

/**
 * @brief PseudoState class used to represent states in transitions that have certain special 
 * traits that require additional information to fully represent, such as additional
 * parent states in direct transitions. These will be specified in the transition table, 
 * and wrapped around the source or target states.
 * 
 */
template <class State, class... ParentStates> class PseudoState {
	public:
		using parent = detail::TypeList<ParentStates...>;
		using state  = State;
		
		constexpr PseudoState( State state__, ParentStates... parent_states_):
			state_(state__),
			parent_states(parent_states_...)
			{}

		constexpr auto getParentState() {
			return mp::mp_front<parent>{};
		}

		constexpr auto getState() {
			return state_;
		}

		constexpr auto getParentStateList(){
			return parent_states;
		}
	
	private:
		parent parent_states;
		State state_;
};

class ExitPseudoState {};
class EntryPseudoState {};
class InitialPseudoState {};
class HistoryPseudoState {};
class DirectPseudoState {};

template <class State, class... ParentState>
class Exit final : public PseudoState<State, ParentState...>, public ExitPseudoState {
	public:
		using parent = detail::TypeList<ParentState...>;
		using state = State;

		constexpr Exit()
			: PseudoState<State,ParentState...>(State{}, ParentState{}...) {}
};


template <class State, class... ParentState>
class Entry final : public PseudoState<State, ParentState...>, public EntryPseudoState {
	public:
		using parent = detail::TypeList<ParentState...>;
		using state = State;

		constexpr Entry()
			: PseudoState<State,ParentState...>(State{}, ParentState{}...) {}
};


template <class State, class... ParentState>
class Direct final : public PseudoState<State, ParentState...>, public DirectPseudoState {
	public:
		using parent = detail::TypeList<ParentState...>;
		using state = State;

		constexpr Direct()
			: PseudoState<State,ParentState...>(State{}, ParentState{}...) {}
};

template <class... ParentState> class History final: public HistoryPseudoState {
	public:
		using parent = detail::TypeList<ParentState...>;

		constexpr History()
			: parent_states(ParentState{}...) {}
	
		constexpr auto getParentState() {
			return mp::mp_front<parent>{};
		}

		constexpr auto getParentStateList(){
			return parent_states;
		}

	private:
		parent parent_states;
};

template <class State> class Initial final: public InitialPseudoState {
	public:
		using state = State;
		constexpr Initial()
			: state_(State {}) {}
		
		constexpr auto getState() {
			return state_;
		}
	
	private:
		State state_;
};

} //namespace sm
} //namespace houdini