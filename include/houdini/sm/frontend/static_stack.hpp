#pragma once

#include <array>
#include <cstddef>
#include <iterator>
#include <iostream>
#include <initializer_list>
#include <utility>

namespace houdini {
namespace sm {

/** \brief A simple wrapper around std::array to emulate 
 * a stack with no dynamic memory allocation.
 * 
 * \todo move constructor current invalidates the end_ptr. 
 * The reason why is unclear - need to investigate. 
 */
template <typename T, std::size_t N>
class StaticStack {
	public:
	
	using StackType = std::array<T, N>;
	using size_type = typename StackType::size_type;
	using difference_type = typename StackType::difference_type;
	using iterator = typename StackType::iterator;
	using const_iterator = typename StackType::const_iterator;
	using reverse_iterator = typename StackType::reverse_iterator;
	using const_reverse_iterator = typename StackType::const_reverse_iterator;

	constexpr StaticStack(){}

	constexpr StaticStack(const StaticStack& other): stack(other.stack) {
		this->end_ptr = this->stack.begin() + other.size();
	}

	constexpr StaticStack(const_iterator begin, const_iterator end){
		while(begin != end){
			this->push_back(*begin);
			begin++;
		}
	}

	constexpr StaticStack(StaticStack&& other) noexcept {
		//TODO: moving the stack can lead to the end_ptr being invalidated. 
		//the reason is unclear. 
		for (auto iter = other.begin(); iter != other.end(); iter++){
			*(this->end_ptr) = std::move(*iter);
			this->end_ptr++;
		}
	}

	constexpr StaticStack operator=(const StaticStack& other){
		return StaticStack(other);
	}

	constexpr StaticStack operator=(StaticStack&& other) noexcept {
		return StaticStack(std::move(other));
	}

	~StaticStack(){}
	
	template <std::size_t V>
	constexpr StaticStack(const std::array<T, V>& array){
		static_assert(V <= N, "Size of copied array exceeds max capacity of stack.");
		for (size_type i = 0; i < array.max_size(); i++){
			if(array[i]){
				this->push_back(array[i]);
			} else {
				break;
			}
		}
		
	}

	constexpr StaticStack(std::initializer_list<T> list){
		static_assert(list.size() <= N, "Number of elements in initializer list exceeds maximum size of stack.");
		for (T&& item:list){
			this->push_back(item);
		}
	}

	// constexpr void push_back(T item) {
	// 	assert(this->end_ptr != this->stack.end() && "Stack overflow");
	// 	*(this->end_ptr) = item;
	// 	this->end_ptr++;
	// }

	constexpr void push_back(T&& item) noexcept {
		assert(this->end_ptr != this->stack.end() && "Stack overflow");
		assert(this->size() <= difference_type(this->stack.max_size()) && "Stack overflow");
		*(this->end_ptr) = std::forward<T>(item);
		this->end_ptr++;
	}

	constexpr void push_back(const T& item) {
		assert(this->end_ptr != this->stack.end() && "Stack overflow");
		assert(this->size() <= difference_type(this->stack.max_size()) && "Stack overflow");
		*(this->end_ptr) = item;
		this->end_ptr++;
	}
	
	/** @brief Push to front of stack. O(n) operation.
	*/
	constexpr void push_front(T&& item) noexcept {
		assert(this->end_ptr != this->stack.end() && "Stack overflow");
		for (size_type i = size_type(this->size()); i > 0; i--){
			this->stack[i] = this->stack[i-1];
		}
		this->stack[0] = std::forward<T>(item);
		this->end_ptr++;
		assert(this->size() <= difference_type(this->stack.max_size()) && "Stack overflow");
	}

	template <typename Type>
	constexpr void emplace_back(Type& item){
		this->push_back({item});
	}

	template <typename Type>
	constexpr void emplace_back(Type&& item){
		this->push_back({item});
	}

	constexpr void pop(){
		assert(this->size() > 0 && "Stack underflow"); 
		this->end_ptr--;
	}

	[[nodiscard]] constexpr T& front() {
		return this->stack.front();
	}

	[[nodiscard]] constexpr const T& front() const {
		return this->stack.front();
	}

	[[nodiscard]] constexpr T& back() {
		return *(this->end_ptr-1);
	}

	[[nodiscard]] constexpr const T& back() const {
		return *(this->end_ptr-1);
	}

	constexpr void clear() {
		this->end_ptr = this->stack.begin();
	}

	constexpr bool empty() const {
		return this->end_ptr == this->cbegin();
	}

	difference_type size() const noexcept {
		difference_type n = std::distance(this->cbegin(), this->cend());
		assert(n >= 0 && "Size of stack is invalid");
		return n;
	}

	constexpr size_type max_size() const noexcept {
		return this->stack.max_size();
	}

	iterator begin(){
		return this->stack.begin();
	}
	
	iterator end(){
		return this->end_ptr;
	}

	constexpr const_iterator cbegin() const {
		return this->stack.cbegin();
	}

	constexpr const_iterator cend() const {
		return const_iterator(this->end_ptr);
	}

	reverse_iterator rbegin() {
		return std::make_reverse_iterator(this->end_ptr);
	}

	reverse_iterator rend(){
		return std::make_reverse_iterator(this->begin());
	}

	constexpr const_reverse_iterator crbegin() const {
		return std::make_reverse_iterator(const_iterator(this->end_ptr));
	}

	constexpr const_reverse_iterator crend() const {
		return std::make_reverse_iterator(this->cbegin());
	}

		StackType stack;
	private:

		iterator end_ptr = stack.begin();
};

}
}