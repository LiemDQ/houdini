#pragma once 
#include <string_view>



namespace houdini {
namespace util {
template <typename T> constexpr std::string_view type_name();

template <> constexpr std::string_view type_name<void>() {
    return "void";
}

namespace detail {
    using type_name_prober = void;

    template <typename T>
    constexpr std::string_view wrapped_type_name() {
        #ifdef __clang__
            return __PRETTY_FUNCTION__;
        #elif defined(__GNUC__)
            return __PRETTY_FUNCTION__;
        #elif defined(_MSC_VER)
            return __FUNCSIG__;
        #else
        #error "Unsupported compiler"
        #endif
    }

    constexpr std::size_t wrapped_type_name_prefix_length() {
        return wrapped_type_name<type_name_prober>().find(type_name<type_name_prober>());
    }

    constexpr std::size_t wrapped_type_name_suffix_length() {
        return wrapped_type_name<type_name_prober>().length()
            - wrapped_type_name_prefix_length()
            - type_name<type_name_prober>().length();
    }
} //namespace detail

/**
 * @brief Utility function that provides the name of a class in a string_View at compile time. 
 */
template <typename T>
constexpr std::string_view type_name() {
    constexpr auto wrapped_name = detail::wrapped_type_name<T>();
    constexpr auto prefix_length = detail::wrapped_type_name_prefix_length();
    constexpr auto suffix_length = detail::wrapped_type_name_suffix_length();
    constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;

    return wrapped_name.substr(prefix_length, type_name_length);
}

template <typename T>
constexpr std::string_view type_name(const T&){
    return type_name<T>();
}

/**
 * Utility classes to make it easier to obtain the type names
 * of various classes. 
 */

/**
 * @brief Helper CRTP interface that stores the derived class's type name in a name data member.
 * Useful for logging.
 */
template <typename ExecutionContext>
struct EnableTypename {
    static constexpr std::string_view static_typename = type_name<ExecutionContext>();
};

/**
 * @brief Helper CRTP interface for classes that are themselves CRTP classes
 * @note If you do not need compile-time type names 
 * it is much easier to use a virtual function instead.
 */
template <typename ExecutionContext, typename Base>
class EnableTypenameDerived: public Base {
    public:
        static constexpr std::string_view static_typename = type_name<ExecutionContext>();
    
    protected:
        constexpr EnableTypenameDerived(): Base(type_name<ExecutionContext>()) {}
        ~EnableTypenameDerived() {}
};

class EnableTypenameBase {
    public:
        std::string_view get_typename() { return name; }
    
    protected:
        constexpr EnableTypenameBase(std::string_view name_): name(name_) {}
        ~EnableTypenameBase() {}
    
    private:
        std::string_view name;
};
} //namespace util
} //namespace houdini