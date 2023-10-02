#include <houdini/sm/sm.hpp>

#include <gtest/gtest.h>

struct S1 {};
struct S2 {};
struct S3 {};
struct S4 {};

struct e1 {};
struct e2 {};
struct e3 {};
struct e4 {};
struct e5 {};
struct e6 {};
struct e7 {};
struct e8 {};
struct e9 {};

//guards 
const auto g1 = [](auto){ return true;};

// Actions 
const auto a1 = [](auto){};

struct SubSubState {
    static constexpr auto make_transition_table() {
        return houdini::transition_table( 
            * houdini::state<S1> + houdini::event<e1> [g1] / a1 = houdini::state<S1>
        );
    }
};

struct SubState {
    static constexpr auto make_transition_table() {
        return houdini::transition_table(
            * houdini::state<S1> + houdini::event<e1> [g1] / a1 = houdini::state<SubSubState>,
              houdini::state<S1> + houdini::event<e2> [g1] / a1 = houdini::state<SubSubState>,
              houdini::state<S1> + houdini::event<e3> [g1] / a1 = houdini::state<SubSubState>,
              houdini::state<S1> + houdini::event<e4> [g1] / a1 = houdini::state<SubSubState>,
              houdini::state<S1> + houdini::event<e5> [g1] / a1 = houdini::state<SubSubState>,
              houdini::state<S1> + houdini::event<e6> [g1] / a1 = houdini::state<SubSubState>,
              houdini::state<S1> + houdini::event<e7> [g1] / a1 = houdini::state<SubSubState>,
              houdini::state<S1> + houdini::event<e8> [g1] / a1 = houdini::state<SubSubState>,
              houdini::state<S1> + houdini::event<e9> [g1] / a1 = houdini::state<SubSubState>
        );
    }
};

struct MainState {
    static constexpr auto make_transition_table() {
        return houdini::transition_table(
            houdini::state<S1> + houdini::event<e1> [g1]/a1 = houdini::state<SubState>,
            houdini::state<S1> + houdini::event<e2> [g1]/a1 = houdini::state<SubState>,
            houdini::state<S1> + houdini::event<e3> [g1]/a1 = houdini::state<SubState>,
            houdini::state<S1> + houdini::event<e4> [g1]/a1 = houdini::state<SubState>,
            houdini::state<S1> + houdini::event<e5> [g1]/a1 = houdini::state<SubState>,
            houdini::state<S1> + houdini::event<e6> [g1]/a1 = houdini::state<SubState>,
            houdini::state<S1> + houdini::event<e7> [g1]/a1 = houdini::state<SubState>,
            houdini::state<S1> + houdini::event<e8> [g1]/a1 = houdini::state<SubState>,
            houdini::state<S1> + houdini::event<e9> [g1]/a1 = houdini::state<SubState>
        );
    }
};

using namespace ::testing;

class PerfTests : public Test {};

