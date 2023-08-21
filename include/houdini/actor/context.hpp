#pragma once

#include "houdini/util/mp11.hpp"
#include "houdini/util/types.hpp"
#include <atomic>

namespace houdini {
namespace act {

enum class ActorStatus {
    IDLE,
    RUN,
    STOP,
    ERROR,
    PANIC
};


struct BaseContext {
        ActorStatus actor_status = ActorStatus::IDLE;
        bool stop_flag = false; 
};

template <typename ExecutionContext>
class Context : public BaseContext {
    
    public:
        Context() = default;
    
    protected:
        ExecutionContext& self() { return static_cast<ExecutionContext&>(*this);}

};



} //namespace act
} //namespace houdini