#pragma once 
#include "houdini/sm/backend/fwd_decl.hpp"
#include "houdini/sm/backend/pseudo_states.hpp"
#include "houdini/sm/backend/transition.hpp"
#include "houdini/sm/backend/event.hpp"

#include "houdini/util/types.hpp"

#include <type_traits>
namespace houdini{
namespace sm {

/**
 * @brief State entry in transition table. This struct uses constexpr operator overloading 
 * to construct transition objects at compile time, the final templated type depending 
 * on the specific features of the transition.
 * 
 */
template <class Type> struct StateBase {
	using type = Type;
	
	template <class Event, JEvent V> constexpr auto operator+(const TEvent<Event, V>&) const {
		return TransitionSE<Type, V> {};
	}

	template <JEvent Event, class Guard>
	constexpr auto operator+(const TransitionEG<Event,Guard>& transition) const {
		return TransitionSEG<Type, Event, Guard> { transition.guard };
	}

	template <JEvent Event, class Action>
	constexpr auto operator+(const TransitionEA<Event,Action>& transition) const {
		return TransitionSEA<Type, Event, Action> {transition.action};
	}

	template <JEvent Event, class Guard, class Action>
	constexpr auto operator+(const TransitionEGA<Event, Guard, Action>& transition) const {
		return TransitionSEGA<Type, Event, Guard, Action> {transition.guard, transition.action};
	}

	template <class Action>
	constexpr auto operator/(const Action& action) const {
		return TransitionSA<Type, Action>{action};
	}

	template <class Guard>
	constexpr auto operator[](const Guard& guard) const {
		return TransitionSG<Type, Guard>{guard};
	}

	template <class Target>
	constexpr auto operator=(const Target&) const {
		return detail::makeTransition<TState<Type>,PLACEHOLDER_NO_EVENT_VALUE,NoGuard,NoAction, Target>{};
	}

	template <class OtherState> bool operator==(const StateBase<OtherState>&) const  
	{
		return std::is_same_v<OtherState, Type>;
	}
};

/** @brief Empty wrapper type used to represent states on the backend for metaprogramming manipulations.
 *  This avoids the need to have states be constructed in the metafunctions, which could complicate constexpr 
 *  computations.
 */
template <class Source> struct TState : public StateBase<Source> {
	using StateBase<Source>::operator=;

	constexpr auto operator*() const {
		return TInitial<Source> {};
	}
};

/**
 * Instantiation of TState.
 */
template <class Source> const TState<Source> state {};

template <class Source> struct TInitial : public StateBase<Initial<TState<Source>>> {
	using StateBase<Initial<TState<Source>>>::operator=;
};
template <class Source> const TInitial<Source> initial {};

template <class State, class... Parents>
struct TDirect : public StateBase<Direct<TState<State>,TState<Parents>...>> {
	using StateBase<Direct<TState<State>,TState<Parents>...>>::operator=;
};

template <class State, class... Parents> const TDirect<State, Parents...> direct {};

template <class State, class... Parents>
struct TEntry : public StateBase<Entry<TState<State>,TState<Parents>...>> {
	using StateBase<Entry<TState<State>,TState<Parents>...>>::operator=;
};
template <class State, class... Parents> const TEntry<State, Parents...> entry {};


template <class State, class... Parents>
struct TExit : public StateBase<Exit<TState<State>,TState<Parents>...>> {
	using StateBase<Exit<TState<State>,TState<Parents>...>>::operator=;
};
template <class State, class... Parents> const TExit<State, Parents...> exit {};

template <class... Parents> struct THistory : public StateBase<History<TState<Parents>...>> {
	using StateBase<History<TState<Parents>...>>::operator=;
};
template <class... Parents> const THistory<Parents...> history {};

} //namespace sm
} //namespace houdini