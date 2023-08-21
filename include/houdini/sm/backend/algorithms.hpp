#pragma once
#include "houdini/util/mp11.hpp"
#include "houdini/util/type_name.hpp"
#include "houdini/sm/backend/index_defs.hpp"
#include "houdini/sm/frontend/static_stack.hpp"

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <string_view>
#include <utility>

namespace houdini {
namespace sm {

template <class First, class Second>
using ToPairs = mp::mp_transform<std::pair, First, Second>;

namespace detail {
template <class First, class Second>
using SizeLessThan = mp::mp_less<mp::mp_size<First>,mp::mp_size<Second>>;

template <class First, class Second>
using SizeGreaterThan = mp::mp_not<SizeLessThan<First,Second>>;

template <class First, class Second>
using TransitionParentSizeLessThan = mp::mp_less<
	mp::mp_size<typename First::parent_t>, mp::mp_size<typename Second::parent_t>>;
} //namespace detail

template <class ListOfLists>
using SizeSort = mp::mp_sort<ListOfLists, detail::SizeLessThan>;

template <class TransitionTable>
using TransitionParentSizeSort = mp::mp_sort<TransitionTable, detail::TransitionParentSizeLessThan>;

template <class ListOfLists>
constexpr auto sizeSortLists(ListOfLists){
	return SizeSort<ListOfLists>{};
}

template <class TransitionTable>
constexpr auto sortTransitionTableByParentSize(TransitionTable){
	return TransitionParentSizeSort<TransitionTable>{};
}

template <class List, class Closure>
inline constexpr auto for_each_index_mp(Closure&& closure){
	std::size_t index = 0;
	mp::mp_for_each<List>(
		[closure, &index](const auto& element){
			closure(element, index);
			index++;
		}
	);
}

template <class StateList, std::size_t MaxDepth> 
constexpr std::array<std::string_view, MaxDepth> get_state_names(){
	constexpr std::size_t list_size = mp::mp_size<StateList>::value;
	static_assert(list_size <= MaxDepth, "Length of state list is longer than max depth of state machine");
	std::array<std::string_view, MaxDepth> arr;
	
	for_each_index_mp<StateList>(
		[&arr](auto state, std::size_t index){
			arr[index] = util::type_name(state);
		}
	);

	return arr; 
}

namespace detail {
template <class StateList, class StateMap, std::size_t MaxIndex, std::size_t MaxDepth>
constexpr StaticStack<StateIndex, MaxDepth> get_state_indices_impl(StaticStack<StateIndex, MaxDepth>& stack){
	if constexpr (mp::mp_empty<StateList>::value){
		return stack; 
	} else {
		StateIndex value = mp::mp_find<StateMap, StateList>::value;
		//std::cout << "Value found: " << value << std::endl;
		assert(value < MaxIndex && "StateList not found in StateMap");
		stack.push_front(std::move(value));
		using FirstEntryRemoved = mp::mp_rest<StateList>;
		return get_state_indices_impl<FirstEntryRemoved, StateMap, MaxIndex, MaxDepth>(stack);
	}
}
} //namespace detail

template <class StateList, class StateMap, std::size_t MaxIndex, std::size_t MaxDepth>
constexpr StaticStack<StateIndex, MaxDepth> get_state_indices(){
	constexpr std::size_t list_size = mp::mp_size<StateList>::value;
	static_assert(list_size <= MaxDepth, "Length of state list is longer than max depth of state machine");
	StaticStack<StateIndex, MaxDepth> stack;
	//std::cout << util::type_name<StateList>() << std::endl;

	return detail::get_state_indices_impl<StateList, StateMap, MaxIndex, MaxDepth>(stack);
}

template <class StateMap> 
constexpr std::array<std::string_view, mp::mp_size<StateMap>::value> get_front_state_name(){
	std::array<std::string_view, mp::mp_size<StateMap>::value> arr;
	for_each_index_mp<StateMap>(
		[&arr](auto state_list, std::size_t index){
			using FirstState = mp::mp_front<decltype(state_list)>;
			arr[index] = util::type_name<FirstState>();
		}
	);

	return arr;
}

template <class Iterable, class Element>
constexpr auto index_of(const Iterable&, const Element&){
	return mp::mp_find<Iterable, Element>{};
}

template <class Iterable, class Element>
constexpr auto index_of_v(const Iterable&, const Element&){
	return mp::mp_find<Iterable, Element>::value;
}

template <class Iterable, class Element>
constexpr auto index_of_mp(){
	return mp::mp_find<Iterable, Element>::value;
}

/**
 * Chain arbitrary number of actions to be used in a transition
 * 
 */
constexpr auto chain_actions = [](auto... actions){
	return [=](auto&&... args){
		return std::apply([&](auto... f){(f(args...), ...);},std::tie(actions...));
	};
};

constexpr auto chain = chain_actions;


} //namespace sm
} //namespace houdini
