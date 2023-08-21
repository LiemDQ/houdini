#pragma once
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <type_traits>
#include <limits>
#include <string_view>
#include <utility>
#include <optional>

#include "houdini/util/types.hpp"
#include "houdini/util/constants.hpp"
#include "houdini/util/utility_functions.hpp"
/**
 @brief Utility functions for enums, particularly for event enums for state machines that use the Janus framework. 
 Based on https://github.com/Neargye/magic_enum. Some features unnecessary for our application were removed, and others were added.

 @note the `IsFlags` bool seen in many of the internal functions indicates whether or
the enum is used as a bit flag; i.e. every bit is a flag and therefore enum values take powers of 2

*/

namespace houdini {

namespace util {


namespace _enum {

using std::string_view;

template <typename T, typename R = void>
using enable_if_enum_t = std::enable_if_t<std::is_enum_v<std::decay_t<T>>, R>;

namespace detail {

/**
 * @brief Default implementation of custom_enum_name. Returns nothing. Custom names for 
 * different enums can be defined by defining a new overload of this function 
 * for that enum type specifically.
 */
template <typename E>
constexpr string_view enum_name(E) noexcept {
    static_assert(std::is_enum_v<E>, "enum_name requires enum type.");
    return {};
}
} //namespace detail

/**
 * @brief gets the enum value name from the __PRETTY_FUNCTION__ string
 * 
 * @par the string passed to this function will have the form 
 * "n<typename E, E V>()[with E = 'EnumType', V = 'EnumType::EnumValue'".
 * Therefore we need to remove everything before the "::", inclusively.
 */
constexpr string_view pretty_name(string_view name) noexcept {
    //
    for (std::size_t i = name.size(); i> 0; --i){
        if (!((name[i-1] >= '0' && name[i-1] <= '9') ||
              (name[i-1] >= 'a' && name[i-1] <= 'z') ||
              (name[i-1] >= 'A' && name[i-1] <= 'Z') ||
              (name[i-1] == '_'))){

            name.remove_prefix(i);
            break;          
        }
    }

    if (name.size() > 0 && ((name.front() >= 'a' && name.front() <= 'z') ||
                            (name.front() >= 'A' && name.front() <= 'Z') ||
                            (name.front() == '_'))) {
        return name;
    }

    return {}; //invalid name
}

/**
 * @brief Compile-time translate an enum value to its name. Only works with Clang or gcc. 
 * 
 * @return The name of the enum value, as a string_view.
 */
template <typename E, E V>
constexpr auto n() noexcept {
    //constexpr auto custom_name = enum_name<E>(V);
#if defined(__clang__) || defined(__GNUC__)
    constexpr auto name = pretty_name({__PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__)-2});
#else
    #error "Unsupported compiler. Must use Clang 5.0+ or gcc 9.0+."
#endif
    return name;
}

/**
 @brief Ensures that the value V actually maps to an enum value for the enum E.
 @return Returns true if V maps to a value in E, otherwise false.
*/
template <typename E, auto V>
constexpr bool is_valid() noexcept {

    static_assert(std::is_enum_v<E>, "_enum::is_valid requires an enum type.");

    return n<E, static_cast<E>(V)>().size() != 0;
}

/**
 @brief returns an enum value i + a constexpr offset O. 
 Typically Used to test the range of validity of an enum type.

*/
template <typename E, int O, bool isFlags, typename U = std::underlying_type<E>>
constexpr E value(std::size_t i) noexcept {
    static_assert(std::is_enum_v<E>, "_enum::value requires an enum type.");

    if constexpr(isFlags){
        return static_cast<E>(U{1} << static_cast<U>(static_cast<int>(i) + O));
    } else {
        return static_cast<E>(static_cast<int>(i) + O);
    }
}

/**
 @brief Returns true if lhs is less than rhs. Otherwise returns false.
  Contains special clauses when lhs and rhs are different types. 
*/
template <typename L, typename R>
constexpr bool cmp_less(L lhs, R rhs) noexcept {
  static_assert(std::is_integral_v<L> && std::is_integral_v<R>, "cmp_less requires integral type.");

  if constexpr (std::is_signed_v<L> == std::is_signed_v<R>) {
    // If same signedness (both signed or both unsigned).
    return lhs < rhs;
  } else if constexpr (std::is_same_v<L, bool>) { // bool special case due to msvc's C4804, C4018
      return static_cast<R>(lhs) < rhs;
  } else if constexpr (std::is_same_v<R, bool>) { // bool special case due to msvc's C4804, C4018
      return lhs < static_cast<L>(rhs);
  } else if constexpr (std::is_signed_v<R>) {
    // If 'right' is negative, then result is 'false', otherwise cast & compare.
    return rhs > 0 && lhs < static_cast<std::make_unsigned_t<R>>(rhs);
  } else {
    // If 'left' is negative, then result is 'true', otherwise cast & compare.
    return lhs < 0 || static_cast<std::make_unsigned_t<L>>(lhs) < rhs;
  }
}


template <typename I>
constexpr I log2(I value) noexcept {
    static_assert(std::is_integral_v<I>, "must be integral type");
    
    I ret = I{0};
    for (; value > I{1}; value >>= I{1}, ++ret){}
    return ret;
}

template <typename I>
constexpr bool is_pow2(I x) noexcept {
    return x != 0 && (x &( x-1)) == 0;
}

/**
 @brief Returns the smallest possible value enum type E could have. 
 @note that this is just the smallest possible value E could theoretically have based on its
 underlying type, not the actual smallest possible value. 

*/
template<typename E, bool IsFlags, typename U = std::underlying_type_t<E>>
constexpr int reflected_min() noexcept {
    static_assert(std::is_enum_v<E>, "_enum::reflected_min requires enum type.");
    
    if constexpr (IsFlags){
        return 0;
    } else {
        constexpr U lhs = JANUS_ENUM_MIN;
        static_assert(std::numeric_limits<std::int16_t>::min(), "Enum minimum must be greater than int16 min");

        constexpr auto rhs = std::numeric_limits<U>::min();

        if constexpr (cmp_less(rhs,lhs)){
            static_assert(!is_valid<E, value<E, lhs-1, IsFlags>(0)>(), "Enum value smaller than min range size detected.");
            return lhs;
        } else {
            return rhs;
        }
    }
}

/**
 @brief Returns the largest possible value enum type E could have. 
 @note This is just the largest possible value E could theoretically have based on its underlying type,
 not the actual smallest possible value.
*/
template<typename E, bool IsFlags, typename U = std::underlying_type_t<E>>
constexpr int reflected_max() noexcept {
    static_assert(std::is_enum_v<E>, "_enum::reflected_min requires enum type.");

    if constexpr (IsFlags){
        return std::numeric_limits<U>::digits-1;
    } else {
        constexpr U lhs = JANUS_ENUM_MAX; //shouldn't exceed user specified maximum
        static_assert(lhs < std::numeric_limits<std::int16_t>::max(), "Enum max must be smaller than int16 max");
        constexpr auto rhs = std::numeric_limits<U>::max();

        if constexpr (cmp_less(lhs, rhs)){
            static_assert(!is_valid<E, value<E, lhs + 1, IsFlags>(0)>(), "Enum value larger than max range size detected.");
            return lhs;
        } else {
            return rhs;
        }
    }
}


template <typename E, bool IsFlags = false>
inline constexpr auto reflected_min_v = reflected_min<E, IsFlags>();

template <typename E, bool IsFlags = false>
inline constexpr auto reflected_max_v = reflected_max<E, IsFlags>();


template <typename T, std::size_t N, std::size_t... I>
constexpr std::array<std::remove_cv_t<T>, N> to_array(T (&a)[N], std::index_sequence<I...>) {
  return {{a[I]...}};
}

/**
 @brief Get number of true values in a C-style bool array
*/
template <std::size_t N>
constexpr std::size_t values_count(const bool (&valid)[N]) noexcept {
    auto count = std::size_t{0};
    for (std::size_t i = 0; i < N; ++i){
        if (valid[i]){
            ++count;
        }
    }

    return count;
}

/**
 @brief Create an array of containing the possible values of the enum E.

 @example enum {FIRST = 0, SECOND = 1, THIRD = 10} -> [0,1,10]
*/
template <typename E, bool IsFlags, int Min, std::size_t... I>
constexpr auto values(std::index_sequence<I...>) noexcept {
    //index sequence contains all possible values that the enum can cover
    //e.g. [0,1,2,3,4,5,6,7,8,9,10]
    //determine if value I maps to a valid enum value
    constexpr bool valid[sizeof...(I)] = {is_valid<E, value<E, Min, IsFlags>(I)>()...}; 
    constexpr std::size_t count = values_count(valid);

    //once we know which values are valid, we group them into a compacted array 
    //so we can easily iterate over it
    if constexpr (count > 0){
        E values[count] = {};
        for (std::size_t i = 0, v=0; v< count; ++i){
            if (valid[i]){
                values[v++] = value<E,Min,IsFlags>(i);
            }
        }
        return to_array(values, std::make_index_sequence<count>{});

    } else {
        //no valid values :(
        return std::array<E, 0>{}; 
    }

}

/**
 @brief Create an array of containing the valid values of the enum E.

 @example enum {FIRST = 0, SECOND = 1, THIRD = 10} -> [0,1,10]
*/
template <typename E, bool IsFlags, typename U = std::underlying_type_t<E>>
constexpr auto values() noexcept {
    //start by determining all possible values that the enum can cover
    constexpr auto min = reflected_min<E, IsFlags>();
    constexpr auto max = reflected_max<E, IsFlags>();
    constexpr auto range_size = max - min +1;
    
    //once full range is determined, find which values map to valid enum values and return array of those
    return values<E, IsFlags, reflected_min_v<E, IsFlags>>(std::make_index_sequence<range_size>{});
}

template <typename E, bool IsFlags = false>
inline constexpr auto values_v = values<E, IsFlags>();

template <typename E, bool IsFlags = false, typename D = std::decay_t<E>>
using values_t = decltype((values_v<D,IsFlags>));

/** @brief Get number of valid values for enum E.
*/
template <typename E, bool IsFlags = false>
inline constexpr auto count_v = values_v<E, IsFlags>.size();

/** @brief Get smallest valid value for enum E.
*/
template <typename E, bool IsFlags = false, typename U = std::underlying_type_t<E>>
inline constexpr auto min_v = (count_v<E,IsFlags> > 0) ? static_cast<U>(values_v<E, IsFlags>.front()) : U{0};

/** @brief Get largest valid value for enum E.
*/
template <typename E, bool IsFlags = false, typename U = std::underlying_type_t<E>>
inline constexpr auto max_v = (count_v<E,IsFlags> > 0) ? static_cast<U>(values_v<E, IsFlags>.back()) : U{0};

template <typename E, E V>
inline constexpr auto enum_name_v = n<E,V>();

/**
 @brief Create an array of string views representing the enum names.
*/
template <typename E, bool IsFlags, std::size_t... I>
constexpr auto names(std::index_sequence<I...>) noexcept {
    //given an array of valid enum values, 
    //we need to iterate over the array and translate each value to its name
    return std::array<string_view, sizeof...(I)>{{enum_name_v<E, values_v<E, IsFlags>[I]>...}};
}

template <typename E, bool IsFlags = false>
inline constexpr auto names_v = names<E, IsFlags>(std::make_index_sequence<count_v<E, IsFlags>>{});

template <typename E, bool IsFlags =false, typename D = std::decay_t<E>>
using names_t = decltype((names_v<D, IsFlags>));

template <typename E, bool IsFlags, typename U = std::underlying_type_t<E>>
constexpr std::size_t range_size() noexcept {
    
    constexpr auto max = IsFlags ? log2(max_v<E, IsFlags>) : max_v<E, IsFlags>;
    constexpr auto min = IsFlags ? log2(min_v<E, IsFlags>) : min_v<E, IsFlags>;
    constexpr auto range_size = max - min + U{1};
    static_assert(range_size > 0, "_enum::enum_range requires valid size.");
    static_assert(range_size < (std::numeric_limits<std::uint16_t>::max)(), "_enum::enum_range requires valid size.");

    return static_cast<std::size_t>(range_size);
}

template <typename E, bool IsFlags = false>
inline constexpr auto range_size_v = range_size<E, IsFlags>();

/** 
 @brief Check if enum skips values in its range of validity
 @example enum {ONE, TWO, THREE} -> false
 @example enum {ONE = 0, TWO = 2, THREE = 3} -> true
*/
template <typename E, bool IsFlags, typename U = std::underlying_type_t<E>>
constexpr bool is_sparse() noexcept {
  static_assert(std::is_enum_v<E>, "is_sparse requires enum type.");

  return range_size_v<E, IsFlags> != count_v<E, IsFlags>;
}

template <typename E, bool IsFlags = false>
using index_t = std::conditional_t<range_size_v<E, IsFlags> < (std::numeric_limits<std::uint8_t>::max)(), std::uint8_t, std::uint16_t>;

template <typename E, bool IsFlags = false>
inline constexpr auto invalid_index_v = (std::numeric_limits<index_t<E, IsFlags>>::max)();

template <typename E, bool IsFlags = false>
inline constexpr bool is_sparse_v = is_sparse<E, IsFlags>();

template <typename E, bool IsFlags, std::size_t... I>
constexpr auto indexes(std::index_sequence<I...>) noexcept {
  static_assert(std::is_enum_v<E>, "_enum::indexes requires enum type.");
  //if enum E skips values then the values in its values_v array cannot be determined by a simple offset.
  //therefore, must create a data structure that reverse maps values to their corresponding indices
  constexpr auto min = IsFlags ? log2(min_v<E, IsFlags>) : min_v<E, IsFlags>;
  [[maybe_unused]] auto i = index_t<E, IsFlags>{0};

  return std::array<decltype(i), sizeof...(I)>{{(is_valid<E, value<E, min, IsFlags>(I)>() ? i++ : invalid_index_v<E, IsFlags>)...}};
}

template <typename E, bool IsFlags = false>
inline constexpr auto indexes_v = indexes<E, IsFlags>(std::make_index_sequence<range_size_v<E, IsFlags>>{});

/** 
 @brief Get the index an enum value in its corresponding value_v array.
*/
template <typename E, typename U = std::underlying_type_t<E>>
constexpr std::size_t undex(U value) noexcept {
    if (const auto i = static_cast<std::size_t>(value - min_v<E>); value>= min_v<E> && value <= max_v<E>){
        if constexpr (is_sparse_v<E>){
            //if the values are sparse, then the indexes cannot be determined using a simple offset 
            //as some values could have been skipped. 
            if (const auto idx = indexes_v<E>[i]; idx != invalid_index_v<E>){
                return idx;
            }
        } else {
            //if values are densely packed (no values are skipped) then start value+offset will correspond to final value.
            return i;
        }
    }

    return invalid_index_v<E>;    
}

/** 
 @brief Get the index of an enum value in its corresponding value_v array.
*/
template <typename E, typename U = std::underlying_type_t<E>>
constexpr std::size_t endex(E value) noexcept {
    return undex<E>(static_cast<U>(value));
}

template <typename E, bool IsFlags = false, std::size_t... I>
constexpr auto entries(std::index_sequence<I...>) noexcept {
    static_assert(std::is_enum_v<E>, "enum::entries requires enum type");

    return std::array<std::pair<E, string_view>, sizeof...(I)>{{{values_v<E, IsFlags>[I], enum_name_v<E, values_v<E, IsFlags>[I]>}...}};
}

template <typename E, bool IsFlags = false>
inline constexpr auto entries_v = entries<E, IsFlags>(std::make_index_sequence<count_v<E, IsFlags>>{});

template <typename E, bool IsFlags = false, typename D = std::decay_t<E>>
using entries_t = decltype((entries_v<D, IsFlags>));

} //namespace _enum

/**
 * @brief Extracts the name of an enum value as a string_view based on its value. Can be performed
 * at compile time. 
 * 
 * @return The name of the enum value as a string_view.
 */
template <typename E>
[[nodiscard]] constexpr auto enum_name(E value) noexcept -> _enum::enable_if_enum_t<E, std::string_view> {
    using D = std::decay_t<E>;
    //it is possible to static_cast an integral type to an enum value
    //without ensuring that it is associated with an enum name.
    //need to ensure that the value provided actually corresponds to a valid enum name
    if (const auto i = _enum::endex<D>(value); i != _enum::invalid_index_v<D>){
        return _enum::names_v<D>[i];
    }

    return {}; //TODO: possible error handling needed?
}

/**
 @brief Get an array containing all of the names of the enum, sorted by underlying value. 
 @example enum {ONE, TWO, THREE} -> ["ONE", "TWO", "THREE"]
 @return std::array<string_view> of all of the names. 
*/
template <typename E>
[[nodiscard]] constexpr auto enum_names() noexcept -> _enum::enable_if_enum_t<E, _enum::names_t<E>> {
    using D = std::decay_t<E>;
    static_assert(_enum::count_v<D> > 0, "Enum names requires enum implementation and valid max and min.");

    return _enum::names_v<D>;
}

template <typename E>
[[nodiscard]] constexpr auto enum_count() noexcept -> _enum::enable_if_enum_t<E, std::size_t>{
    using D = std::decay_t<E>;

    return _enum::count_v<D>;
}

template <typename E>
[[nodiscard]] constexpr auto enum_value(E v) noexcept -> _enum::enable_if_enum_t<E, std::underlying_type_t<E>>{
    using D = std::decay_t<E>;
    using R = std::underlying_type_t<D>;

    return static_cast<R>(v);
}

/** 
 @brief Returns an array of the values of the enum, sorted by value. 
 @return `std::array` of values 
*/
template <typename E>
[[nodiscard]] constexpr auto enum_values() noexcept -> _enum::enable_if_enum_t<E, _enum::values_t<E>> {
    using D = std::decay_t<E>;
    static_assert(_enum::count_v<D> > 0, "Enum values requires enum implementation and valid max and min.");

    return _enum::values_v<D>;
}

template <typename E>
[[nodiscard]] constexpr auto enum_underlying_values() noexcept {
    using D = std::decay_t<E>;
    static_assert(_enum::count_v<D> > 0, "Enum underlying values requires enum implementation and valid max and min.");

    constexpr auto enum_vals = _enum::values_v<D>;
    using U = std::underlying_type_t<D>;
    using OutputArray = std::array<U, enum_vals.max_size()>;

    OutputArray output_array;
    util::transform(enum_vals.cbegin(), enum_vals.cend(), output_array.begin(), [](D val){return static_cast<U>(val);} );

    return output_array;
}

template <typename E>
[[nodiscard]] constexpr auto enum_max_value() noexcept -> _enum::enable_if_enum_t<E, std::underlying_type_t<E>>{
    using U = std::underlying_type_t<E>;

    auto value_array = enum_values<E>();
    return static_cast<U>(value_array.back());
}

template <typename E>
[[nodiscard]] constexpr auto enum_min_value() noexcept -> _enum::enable_if_enum_t<E, std::underlying_type_t<E>>{
    using U = std::underlying_type_t<E>;
    
    auto value_array = enum_values<E>();
    return static_cast<U>(value_array.front()); //values are sorted by value
}

template <typename E>
[[nodiscard]] constexpr bool enum_value_valid(E value){
    using D = std::decay_t<E>;
    if (const auto i = _enum::endex<D>(value); i != _enum::invalid_index_v<D>){
        return true;
    } 
    return false;
}

template <typename E>
[[nodiscard]] constexpr auto enum_from_str(std::string_view str) -> std::optional<E> {
    constexpr auto values = enum_values<E>();
    
    for (E value:values){
        if (enum_name(value) == str){
            return value;
        }
    }
    return std::nullopt;
}

} //namespace util
} //namespace houdini