#pragma once
#include "houdini/sm/backend/type_list.hpp"

namespace houdini {
namespace sm {

constexpr auto transition_table = detail::makeTypeList;

template <class... Events> constexpr auto events = detail::TypeList<Events...>{}; 

}
}