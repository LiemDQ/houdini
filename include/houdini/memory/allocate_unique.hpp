#pragma once
#include <memory>
#include <type_traits>

namespace houdini {
namespace memory {
namespace detail {

/**
 * Most generic implementation of a deleter that takes a memory allocator 
 * that conforms to std::allocator traits. 
 */
template <typename Alloc>
struct AllocDeleter {
    AllocDeleter(const Alloc& a): a(a) { }

    using pointer = typename std::allocator_traits<Alloc>::pointer;

    void operator()(pointer p) const {
        Alloc aa(a);
        std::allocator_traits<Alloc>::destroy(aa, std::addressof(*p));
        std::allocator_traits<Alloc>::deallocate(aa, p, 1);
    }
    private:
        Alloc a;
};
} //namespace 

/**
 * Utility function to create a std::unique_ptr with custom memory allocation.
 * For API consistency with std::allocate_shared.
 */
template <class T, class Alloc, class... Args>
inline std::unique_ptr<T> allocate_unique(const Alloc& alloc, Args&&... args) {
    using AT = std::allocator_traits<Alloc>;
    static_assert(std::is_same_v<typename AT::value_type, std::remove_cv_t<T>>, 
            "Allocator has the wrong value_type");
    
    Alloc a(alloc);
    auto p = AT::allocate(a, 1);
    try {
        AT::construct(a, std::addressof(*p), std::forward<Args>(args)...);
        using D = detail::AllocDeleter<Alloc>;
        return std::unique_ptr<T,D>(p, D(a));
    } catch (...) {
        AT::deallocate(a, p, 1);
        throw;
    }
}


} // namespace memory 
} // namespace houdini 