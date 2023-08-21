#pragma once
#include <houdini/util/mp11.hpp>

/**
 * mp_flatten is only available in boost version 1.73.0 onwards, which does not 
 * ship with Ubuntu 20.04 by default.
 * This is an implementation that can be used with the default version of boost.
 */
namespace houdini {
namespace util {
namespace detail {
    template <class L2> struct mp_flatten_impl {
        template <class T> using fn = mp::mp_if<mp::mp_similar<L2,T>,T,mp::mp_list<T>>;
    };
} //namespace detail

template <class L, class L2 = mp::mp_clear<L>> 
using mp_flatten = mp::mp_apply<mp::mp_append, mp::mp_push_front<mp::mp_transform_q<
    detail::mp_flatten_impl<L2>,L>, mp::mp_clear<L>>>;
} //namespace util
} //namespace