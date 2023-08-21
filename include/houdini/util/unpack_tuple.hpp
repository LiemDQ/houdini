#pragma once
#include <tuple>
#include <utility>
#include <type_traits>

namespace houdini {
namespace util {
namespace detail {

template <typename F, typename Tuple, bool Done, int Total, std::size_t... N>
struct unpack_impl {
    static constexpr auto unpack(F f, Tuple&& t){
        return unpack_impl<F, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>
        ::unpack(f, std::forward<Tuple>(t));
    }
};



template <typename F, typename Tuple, int Total, std::size_t... N>
struct unpack_impl<F, Tuple, true, Total, N...>{
    static constexpr auto
    unpack(F f, Tuple&& t) -> std::invoke_result_t<F, decltype(std::get<N>(std::forward<Tuple>(t)))...> {
        //NOTE: forwarding inside a parameter pack expansion is usually undefined behavior, 
        //as values can only be moved once in a scope! However, this works because std::get(tuple<Types...>&& t)
        //is explicitly specified to move the specified element, and NOT the tuple. 
        return f(std::get<N>(std::forward<Tuple>(t))...);
    }
};
} // namespace detail

/**
 * @brief Accepts a function and a tuple, and forwards all the entries in the tuple to the function.
 * Designed to work with parameter pack functions. 
 */
template <typename F, typename Tuple>
constexpr auto unpack(F f, Tuple&& t){
    using Type = std::decay_t<Tuple>;
    return detail::unpack_impl<F, Tuple, 0 == std::tuple_size_v<Type>, std::tuple_size_v<Type>>
    ::unpack(f, std::forward<Tuple>(t));
}

} // namespace util
} // namespace houdini