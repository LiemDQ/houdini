#pragma once
#include "houdini/houdini.hpp"
#include "houdini/util/types.hpp"

using MyState = houdini::State<houdini::act::BaseContext, houdini::brokers::BaseBroker>;

enum Events : houdini::JEvent {
    e1,
    e2,
    e3,
    ie1,
    ie2,
    ie3,
    iie1,
    iie2,
    iie3
};

struct SSS1 : MyState {};
struct SSS2 : houdini::State<> {};
struct SSS3 : houdini::State<> {};

struct SS1 : houdini::State<> {};
struct SS2 : houdini::State<> {};
struct SS3 : houdini::State<> {};

struct S1 : houdini::State<> {};
struct S2 : houdini::State<> {};
struct S3 : houdini::State<> {};

JANUS_CREATE_EVENT(Events, event);

struct Root : houdini::State<> {
    static constexpr auto make_transition_table(){
        using namespace houdini;
        //clang-format off
        return houdini::transition_table(
            *state<S1> + event<e1> = state<S2>,
             state<S2> + event<e1> = state<S3>,
             state<S3> + event<e1> = state<S1>
        );
        //clang-format on
    }
};



