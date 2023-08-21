#pragma once
#include "houdini/sm/backend/event.hpp"
#include "houdini/sm/sm.hpp"

/** @brief Basic 2-level state machine that can be reused for basic 
unit tests. Simply include this header along with gtest.h
and construct a fixture class to use it.
*/

struct IS21 : houdini::State<> {};
struct IS22 : houdini::State<> {};
struct IS23 : houdini::State<> {};

struct IS31 : houdini::State<> {};
struct IS32 : houdini::State<> {};
struct IS33 : houdini::State<> {};


struct S1 : houdini::State<> {

};

struct FalseGuard {
    template <typename Context, typename Broker>
    bool operator()(houdini::JEvent, Context, Broker) const {
        return false;
    }

};

struct TrueGuard {
    template <typename Context, typename Broker>
    bool operator()(houdini::JEvent, Context, Broker) const {
        return true;
    }
};

enum Events : houdini::JEvent {
    e1,
    e2,
    e3,
    e4,
    ie1,
    ie2
};

JANUS_CREATE_EVENT(Events, event);

struct S2 : houdini::State<> {
    static constexpr auto make_transition_table(){
        //clang-format off
        using namespace houdini;
        return houdini::transition_table(
            *state<IS21> + event<ie1> = state<IS22>,
             state<IS22> + event<ie2> = state<IS23>,
             state<IS23> + event<e3>  = state<IS21>,
             state<IS21> + event<e3>  = state<IS23>
        );
        //clang-format on
    }

};


struct S3 : houdini::State <> {
    static constexpr auto make_transition_table(){
        //clang-format off
        using namespace houdini;
        return houdini::transition_table(
            *state<IS31> + event<ie1> = state<IS32>,
             state<IS32> + event<ie2> = state<IS33>,
             state<IS33> + event<e3>  = state<IS31>,
             state<IS31> + event<e3>  = state<IS33>
        );
        //clang-format on
    }

};

struct S4 : houdini::State<> {};

struct Root : houdini::State<> {
    static constexpr auto make_transition_table(){
        //clang-format off
        using namespace houdini;
        return houdini::transition_table(
            *state<S1> + event<e1>[TrueGuard{}] = state<S2>,
             state<S2> + event<e2> = state<S3>,
             state<S2> + event<e3> = state<S3>,
             state<S3> + event<e2> = state<S4>,
             state<S3> + event<e1> = state<S2>,
             state<S1> + event<e4>[TrueGuard{}] = state<S4>,
             state<S1> + event<e3>[FalseGuard{}] = state<S4>,
             state<S2> + event<e4> = state<S2>,
             state<S4> + event<e4> = state<S4>
        );
        //clang-format on
    }

};

