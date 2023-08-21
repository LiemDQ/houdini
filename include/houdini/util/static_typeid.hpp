#pragma once

namespace houdini {
namespace util {
namespace detail {

template <typename T>
struct TypeidPtr {
	//having a static data member will ensure each type corresponds to a unique address in memory
	static const T* const id;
};

template <typename T>
const T* const TypeidPtr<T>::id = nullptr;

} //namespace detail

using TypeidType = const void*;

/**
 * @brief Lightweight, constexpr implementation of RTTI without having to enable RTTI in compilation. 
 * Allows each type to be assigned a unique ID. Can also be used at compile time, for example in constexpr data containers.
 */
template <typename T>
constexpr TypeidType type_id() {return &detail::TypeidPtr<T>::id;}

/**
 * @brief Lightweight, constexpr implementation of RTTI without having to enable RTTI in compilation. 
 * Allows each type to be assigned a unique ID. Can also be used at compile time, for example in constexpr data containers.
 */
template <typename T>
constexpr TypeidType type_id(T x) {return &detail::TypeidPtr<decltype(x)>::id;}

/**
 * @brief Utility interface that can be inherited from to enable "RTTI" through the get_id() function.
 */
template <typename Derived>
struct EnableRtti {
	static constexpr TypeidType type_id = ::houdini::util::type_id<Derived>();
};


/**
 * @brief RTTI interface for CRTP classes. The derived class should inherit from this, 
 * templated by itself and the base class. The inheritance hierarchy will resolve to
 * Derived -> EnableRttiDerived -> Base -> EnableRttiBase
 */
template <typename Derived, typename Base>
class EnableRttiDerived : public Base {
	public:
		static constexpr TypeidType get_static_id() noexcept {
			return ::houdini::util::type_id<Derived>();
		}

	protected:
		EnableRttiDerived(): Base(::houdini::util::type_id<Derived>()) {}
		~EnableRttiDerived() {}
};

/**
 * @brief RTTI interface for bases of CRTP classes. 
 * Oftentimes, you will want to refer to a non-templated base class, as the type of templated classes (CRTP) is hard,
 * if not impossible to deduce. However, we need to pass on the information of the derived class to enable RTTI
 * somehow, through a nontemplated manner. We achieve this by passing the information about the derived class
 * through a constructor, and storing it in a member variable.
 * 
 * @par The base class must inherit from this class in order to enable RTTI. It must also forward the id_provider argument
 * to EnableRttiBase, through its constructor. 
 */
class EnableRttiBase {
	public:
		TypeidType get_id() const noexcept {
			return id_provider;
		}

	protected:
		EnableRttiBase(TypeidType id_provider_): id_provider(id_provider_) {}
		~EnableRttiBase() {}
	
	private:
		TypeidType id_provider;
};

} //namespace util
} //namespace houdini