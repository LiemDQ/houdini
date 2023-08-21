#pragma once
#include <variant>
#include <utility>
#include <functional>
#include <type_traits>
#include <memory>

namespace houdini {
namespace util {

template <typename... Fs>
struct overload : Fs... {
    using Fs::operator()...;
};

template <typename... Fs> 
overload(Fs...) -> overload<Fs...>; //C++17 deduction guide, not needed in C++20

/**
 * @brief Faster implementation of std::visit as long as we can guarantee the variant will never be empty.
 * Has the same overhead as a virtual function call. 
 */
template <std::size_t N, typename R, typename Variant, typename Visitor>
[[nodiscard]] constexpr R visit(Variant&& var, Visitor&& vis){
    if constexpr (N ==0){
        if (N == var.index()){
        // if this check isn't here the compiler will generate exception code
            return std::forward<Visitor>(vis)(
                std::get<N>(std::forward<Variant>(var)));
        }
    }
    else {
        if (var.index() == N){
            return std::forward<Visitor>(vis)(
                std::get<N>(std::forward<Variant>(var)));
        }
        //the recursion is checked at compile time and is guaranteed to terminate 
        return visit<N-1,R>(std::forward<Variant>(var),
                            std::forward<Visitor>(vis));
    }
}

/**
 * @brief Faster implementation of `std::visit` as long as we can guarantee the variant
 * will never be empty. The API differs slightly from `std::visit`. Supports multiple visitors. 
 * 
 * @note The visitor chosen to be applied to each variant's arguments will be the closest match
 * according to standard overload resolution rules. See https://en.cppreference.com/w/cpp/language/overload_resolution. 
 * 
 * @param var The variant to be visited.
 * @param vis Function used to visit the variant. 
 * @param visitors Additional
 * 
 * @return The return value of the visitor function applied to the contents of the variant.
 */
template <class... Args, typename Visitor, typename... Visitors>
[[nodiscard]] constexpr decltype(auto) visit(std::variant<Args...> &var, Visitor&& vis, Visitors&&... visitors){
    
    //overload class has an overloaded ()operator and 
    //allows for the most appropriate visitor callable to be invoked based 
    //on standard overload resolution rules
    auto o1 = overload{std::forward<Visitor>(vis), std::forward<Visitors>(visitors)...};
    using result_t = decltype(std::invoke(std::move(o1), std::get<0>(var)));

    static_assert(sizeof...(Args) > 0);
    return visit<sizeof...(Args) - 1, result_t>(var, std::move(o1));
}

/**
 * @brief Faster implementation of std::visit as long as we can guarantee the variant
 * will never be empty. The API differs slightly from `std::visit`. Supports multiple visitors.
 * Overload for rvalue variants (i.e. constructed in place, or moved)
 * 
 * @note The visitor chosen to be applied to each variant's arguments will be the closest match
 * according to standard overload resolution rules. See https://en.cppreference.com/w/cpp/language/overload_resolution. 
 * 
 * @param var The variant to be visited.
 * @param vis Function used to visit the variant. 
 * @param visitors Additional
 * 
 * @return The return value of the visitor function applied to the contents of the variant.
 */
template <class... Args, typename Visitor, typename... Visitors>
[[nodiscard]] constexpr decltype(auto) visit(std::variant<Args...>&& var, Visitor&& vis, Visitors&&... visitors){
    
    //overload class has an overloaded ()operator and 
    //allows for the most appropriate visitor callable to be invoked based 
    //on standard overload resolution rules
    auto o1 = overload{std::forward<Visitor>(vis), std::forward<Visitors>(visitors)...};

    using result_t = 
        decltype(std::invoke(std::move(o1), std::move(std::get<0>(var))));

    static_assert(sizeof...(Args) > 0);
    return visit<sizeof...(Args) - 1, result_t>(std::move(var), std::move(o1));
}

template <typename Value, typename... Visitors>
inline constexpr bool is_visitable_v = (std::is_invocable_v<Visitors,Value> or ...);

} // namespace util
} // namespace houdini