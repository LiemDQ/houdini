#pragma once

#include "houdini/util/types.hpp"

#include <rclcpp/rclcpp.hpp>

#include <cstdint>

namespace houdini {

enum class DefaultBrokerError : JErrorFlag {
    NONE = 0
};

enum class CommandStatus : JErrorFlag {
    CMD_NOT_FOUND,
    STALE, 
    FALSE,
    TRUE
};


}