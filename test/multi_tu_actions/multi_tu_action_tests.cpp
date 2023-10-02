#include "houdini/houdini.hpp"
#include "state_1.hpp"
#include "state_2.hpp"
#include "action_context.hpp"
#include <gtest/gtest.h>

struct RootSM : houdini::State<TestContext, Broker> {
    static constexpr auto make_transition_table() {
        using namespace houdini;
        return transition_table(
            *state<State1> + event<e1> = state<State2>,
             state<State2> + event<e2> = state<State1>
        );
    }
};

class MultiTUActionTests : public ::testing::Test {
    protected:
        TestContext context;
        Broker broker;
        houdini::SM<RootSM, Events, TestContext, Broker> state_machine{context, broker};

};

TEST_F(MultiTUActionTests, stateEntryActionsAreSuccessful){
    state_machine.processEvent(e1);
    ASSERT_EQ(context.l2, 2);
    ASSERT_EQ(context.l1, 4);
}

TEST_F(MultiTUActionTests, stateExitActionsAreSuccessful){
    state_machine.processEvent(e1);
    state_machine.processEvent(e2);
    ASSERT_EQ(context.l2, 1);
    ASSERT_EQ(context.l1, 16);
}