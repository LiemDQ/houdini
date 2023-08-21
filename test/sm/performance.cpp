#include <estate/sm/sm.hpp>

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
        return estate::transition_table( 
            * estate::state<S1> + estate::event<e1> [g1] / a1 = estate::state<S1>
        );
    }
};

struct SubState {
    static constexpr auto make_transition_table() {
        return estate::transition_table(
            * estate::state<S1> + estate::event<e1> [g1] / a1 = estate::state<SubSubState>,
              estate::state<S1> + estate::event<e2> [g1] / a1 = estate::state<SubSubState>,
              estate::state<S1> + estate::event<e3> [g1] / a1 = estate::state<SubSubState>,
              estate::state<S1> + estate::event<e4> [g1] / a1 = estate::state<SubSubState>,
              estate::state<S1> + estate::event<e5> [g1] / a1 = estate::state<SubSubState>,
              estate::state<S1> + estate::event<e6> [g1] / a1 = estate::state<SubSubState>,
              estate::state<S1> + estate::event<e7> [g1] / a1 = estate::state<SubSubState>,
              estate::state<S1> + estate::event<e8> [g1] / a1 = estate::state<SubSubState>,
              estate::state<S1> + estate::event<e9> [g1] / a1 = estate::state<SubSubState>
        );
    }
};

struct MainState {
    static constexpr auto make_transition_table() {
        return estate::transition_table(
            estate::state<S1> + estate::event<e1> [g1]/a1 = estate::state<SubState>,
            estate::state<S1> + estate::event<e2> [g1]/a1 = estate::state<SubState>,
            estate::state<S1> + estate::event<e3> [g1]/a1 = estate::state<SubState>,
            estate::state<S1> + estate::event<e4> [g1]/a1 = estate::state<SubState>,
            estate::state<S1> + estate::event<e5> [g1]/a1 = estate::state<SubState>,
            estate::state<S1> + estate::event<e6> [g1]/a1 = estate::state<SubState>,
            estate::state<S1> + estate::event<e7> [g1]/a1 = estate::state<SubState>,
            estate::state<S1> + estate::event<e8> [g1]/a1 = estate::state<SubState>,
            estate::state<S1> + estate::event<e9> [g1]/a1 = estate::state<SubState>
        );
    }
};

using namespace ::testing;

class PerfTests : public Test {};

