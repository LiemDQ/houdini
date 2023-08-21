#pragma once
//#include <houdini/memory/tlsf_resource.hpp>
#include <limits>
#include <vector>
#include <queue>
#include <unordered_map>
#include <memory_resource>
#include <variant>
#include <chrono>
#include <utility>
#include <cstdint>

namespace houdini {

template <typename T>
using JAllocator = std::pmr::polymorphic_allocator<T>;

template <typename K, typename V, typename Hash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
using JUnorderedMap = std::unordered_map<K,V,Hash,KeyEqual,JAllocator<std::pair<const K,V>>>;

template <typename T>
using JVector = std::pmr::vector<T>;

template <typename T>
using JQueue = std::queue<T>;

template <typename... T>
using JVariant = std::variant<T...>;

using msTimeDuration = std::chrono::milliseconds;
using usTimeDuration = std::chrono::microseconds;
using TimeDuration = msTimeDuration;

using JEvent = uint16_t;
using JCommand = JEvent;
using JErrorFlag = uint16_t;

constexpr JEvent JEVENT_MAX = std::numeric_limits<JEvent>::max();

}