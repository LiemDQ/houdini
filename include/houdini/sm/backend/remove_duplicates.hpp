#pragma once
#include "houdini/sm/backend/event.hpp"

#include "houdini/util/mp11.hpp"
#include "houdini/util/utility_functions.hpp"
#include "houdini/util/types.hpp"

#include <limits>

namespace houdini {
namespace sm {

template <class Tuple> constexpr auto removeDuplicates(Tuple){
	return mp::mp_unique<std::decay_t<Tuple>> {};
}

template <class Array> constexpr auto removeArrayDuplicatesAndFillWithMax(Array arr){
	//constexpr JEvent max_value = std::numeric_limits<JEvent>::max();

	auto arr2 = util::fill_array(arr, JEVENT_MAX);

	auto isInArray = [](auto& array, JEvent value){
		for (auto&& item: array){
			if (item == value)
				return true;
		}
		return false;
	};

	std::size_t i = 0;
	for (auto&& item: arr){
		if (isInArray(arr2, item)){
			continue;
		} else {
			arr2[i] = item;
			i++;
		}
	}
	return arr2;
}

}
}