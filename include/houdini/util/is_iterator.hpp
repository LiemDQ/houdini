#pragma once
#include <type_traits>
#include <iterator>

namespace houdini {
namespace util {

/** @brief Checks if a type is an iterator, through compliance with std::iterator_traits.
*/
template <typename T>
struct is_iterator {
    static char test(...);

    template <typename U,
        typename=typename std::iterator_traits<U>::difference_type,
        typename=typename std::iterator_traits<U>::pointer,
        typename=typename std::iterator_traits<U>::reference,
        typename=typename std::iterator_traits<U>::value_type,
        typename=typename std::iterator_traits<U>::iterator_category
    > static long test(U&&);

    constexpr static bool value = std::is_same<decltype(test(std::declval<T>())), long>::value;
    using type = typename std::is_same<decltype(test(std::declval<T>())), long>::type;
};

template <typename T>
constexpr bool is_iterator_v = is_iterator<T>::value;

template <typename T>
using is_iterator_t = typename is_iterator<T>::type;

}
}