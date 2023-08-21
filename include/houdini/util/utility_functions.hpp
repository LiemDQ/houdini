#pragma once
#include <algorithm>
#include <type_traits>
#include <array>

namespace houdini {
namespace util {

template <typename T>
struct is_array: std::is_array<T>{};

//std::is_array returns false for std::array.
//this metafunction fixes that
template <class T, std::size_t N>
struct is_array<std::array<T,N>> : std::true_type {}; 

/**
 * @brief Creates an array type filled with a specified value. Can be used at compile time.
 * 
 * @note std::array.fill() is constexpr only for C++20 onwards. This function 'fills'
 * the gap for C++17 and under. 
 */
template <typename T, std::size_t N, template <typename, std::size_t> typename Array> 
constexpr Array<T,N> fill_array(const Array<T,N>& arr, T value){
	using ArrayT = Array<T,N>;
	static_assert(is_array<ArrayT>::value, "Must be array type");
	ArrayT arr2{};
	for (std::size_t i = 0; i < arr.size(); ++i){
		arr2[i] = value;
	}
	return arr2; 
}

/**
 @brief constexpr version of std::transform (std::transform is only constexpr from C++20 onwards)
*/
template <typename InputIt, typename OutputIt, typename UnaryOperation>
constexpr OutputIt transform(InputIt i_first, InputIt i_last, 
							OutputIt o_first, UnaryOperation unary_op){
	while(i_first != i_last){
		*o_first = unary_op(*i_first);
		i_first++;
		o_first++;
	}
	return o_first;
}

/**
 @brief constexpr version of std::transform (std::transform is only constexpr from C++20 onwards)
*/
template <typename InputIt1, typename InputIt2, typename OutputIt, typename BinaryOperation>
constexpr OutputIt transform(InputIt1 i_first1, InputIt1 i_last, InputIt2 i_first2, 
							OutputIt o_first, BinaryOperation binary_op){
	while(i_first1 != i_last){
		*o_first = binary_op(*i_first1, *i_first2);
		i_first1++;
		i_first2++;
		o_first++;
	}
	return o_first;
}

}
}