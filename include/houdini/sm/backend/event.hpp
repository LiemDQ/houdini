#pragma once
#include "houdini/sm/backend/fwd_decl.hpp"
#include "houdini/sm/backend/pseudo_states.hpp"
#include "houdini/sm/backend/transition.hpp"

#include "houdini/util/static_typeid.hpp"
#include "houdini/util/types.hpp"
#include "houdini/util/enum_utils.hpp"

#include <type_traits>
#include <limits>

namespace houdini {
namespace sm {

//TODO: determine whether this should be different value instead
//note that this value cannot exceed the maximum static_sm map array size or 
//the program will fail to compile
constexpr JEvent PLACEHOLDER_NO_EVENT_VALUE = JEVENT_MAX-1;

struct NoEvent{};

/**
 * @brief Event class used internally by the state machine in order to resolve 
 * transitions at compile time. Events do not need to inherit or otherwise be associated 
 * with this class. 
 */
template <class EventEnum, JEvent EnumValue> 
struct TEvent {
	//static constexpr util::TypeidType type_id = util::type_id<Event>();
	using type = EventEnum;
	static_assert(std::is_enum_v<EventEnum> || std::is_same_v<EventEnum, NoEvent>, 
		"EventEnum must be enum type or NoEvent");
	
	//static_assert(EnumValue > 0, "Event enum cannot have a value of 0. Please ensure the enum starts at 1.");

	constexpr auto operator+() const {
		return detail::makeInternalTransition<EnumValue, NoGuard, NoAction>{};
	}

	template <class Guard> constexpr auto operator[](const Guard& guard) const {
		return TransitionEG<EnumValue, Guard>{guard};
	}

	template <class Action> constexpr auto operator/(const Action& action) const {
		return TransitionEA<EnumValue, Action>{action};
	}

};


template <class EventEnum, EventEnum V>
using ConvertEvent = TEvent<EventEnum, static_cast<JEvent>(V)>;

using none = TEvent<NoEvent, std::numeric_limits<JEvent>::max()>; //placeholder value

#define JANUS_CREATE_EVENT(EnumType, event_name) template<EnumType value> houdini::sm::ConvertEvent<EnumType, value> event_name

} //namespace sm
} //namespace houdini