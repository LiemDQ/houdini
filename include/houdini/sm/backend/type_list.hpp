#pragma once
#include "houdini/util/mp11.hpp"

#include <type_traits>

namespace houdini {
namespace sm {
namespace detail {

//type alias makes the transition type list easy to switch later
template <class... Xs> 
using TypeList = mp::mp_list<Xs...>;

constexpr auto makeTypeList = [](auto&&... xs){
	return TypeList<std::decay_t<decltype(xs)>...> {};
};

} //namespace detail
} //namespace sm
} //namespace houdini