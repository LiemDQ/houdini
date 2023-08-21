#pragma once

#include "houdini/util/mp11.hpp"
#include "houdini/util/types.hpp"

#include <stdexcept>
#include <queue>
#include <variant>

namespace houdini {
namespace sm {
namespace detail {
template <class EventsTuple> auto constexpr makeVariantQueue(EventsTuple){
	using TVariant = mp::mp_rename<EventsTuple, std::variant>;
	
	return std::queue<TVariant>{};
}
}

/**
 * @brief Variant queue is a wrapper class around a queue container
 * that creates a variant value type from the tuple that is passed to it.
 * It is needed for dispatching deferred events, where the events could all have different
 * types but need to held in the same queue. 
 */
template <class EventsTuple>
class VariantQueue {
	EventsTuple events;
	using TQueue = decltype(detail::makeVariantQueue(events));
	TQueue queue;

	public:
		VariantQueue(const EventsTuple& events_): events(events_) {}

		[[nodiscard]] auto empty() const -> bool {
			return this->queue.empty();
		}

		[[nodiscard]] auto size() const -> bool {
			return this->queue.size();
		}

		template <class T> void push(const T& e){
			this->queue.push(e);
		}

		template <class T> void push(T&& e){
			this->queue.push(e);
		}

		template <class Callable>
		void visit(const Callable& callable){
			if (empty()){
				throw std::runtime_error("Variant queue is empty"); //TODO: check if we want this or not or if we prefer having an error code
			}

			auto front_element = this->queue.front();
			this->queue.pop();

			std::visit([&callable](auto&& arg){
				callable(arg);
			}, front_element);

		}
};

/**
 * @brief Wrapper class around std::queue. Used to 
 * restrict the public interface of the queue and facilitate
 * using it for its intended purpose: applying events to callable functions.
 */
class DeferQueue {
	using TQueue = std::queue<JEvent>;
	TQueue queue;

	public:
		
		[[nodiscard]] auto empty() const -> bool {
			return this->queue.empty();
		}

		[[nodiscard]] auto size() const -> bool {
			return this->queue.size();
		}

		template <class T> void push(const T& e){
			this->queue.push(e);
		}

		template <class T> void push(T&& e){
			this->queue.push(e);
		}

		template <class Callable>
		void visit(const Callable& callable){
			if (this->empty()){
				throw std::runtime_error("Variant queue is empty"); //TODO: check if we want this or not or if we prefer having an error code
			}

			auto front_element = this->queue.front();
			this->queue.pop();
			callable(front_element);
		}
};

} //namespace sm
} //namespace houdini