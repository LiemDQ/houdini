#include "basic_sm.hpp"
#include "houdini/actor/context.hpp"
#include "houdini/sm/backend/algorithms.hpp"
#include "houdini/sm/backend/collect.hpp"
#include "houdini/sm/backend/collect_initial_states.hpp"
#include "houdini/sm/backend/flatten.hpp"
#include "houdini/sm/backend/index.hpp"
#include "houdini/sm/backend/resolve_state.hpp"
#include "houdini/util/type_name.hpp"
#include <gtest/gtest.h>

class BasicTransitionTests : public ::testing::Test {
    protected:
        houdini::act::BaseContext context;
        houdini::brokers::BaseBroker broker;
        houdini::SM<Root, Events> state_machine{context, broker};

};


TEST_F(BasicTransitionTests, stateMachineInitializesCorrectly){
    using ParentList = houdini::sm::detail::TypeList<houdini::sm::TState<Root>>;
    //auto ists = houdini::sm::makeInitialStateMap(state_machine.root_state);

    EXPECT_EQ(1, houdini::sm::getCombinedStateIndex(houdini::sm::getCombinedStateTypeIDs(state_machine.root_state), 
        ParentList{}, houdini::state<S1>)) << "Combined state table is correctly formulated?";

    EXPECT_EQ(1, state_machine.currentState()) << "Initial state index is correct?";
    EXPECT_EQ(state_machine.currentStateName(), "S1") << "Initial state name is initialized correctly?";
    EXPECT_TRUE(state_machine.is(houdini::state<S1>));// << houdini::util::type_name(houdini::sm::getCombinedStateTypeIDs(state_machine.root_state));    
    
}

TEST_F(BasicTransitionTests, transitionToSameLevelState){
    EXPECT_TRUE(state_machine.is(houdini::state<S1>));
    state_machine.processEvent(e4);
    EXPECT_EQ(state_machine.currentStateName(), "S4");
    EXPECT_TRUE(state_machine.is(houdini::state<S4>));
        
}

TEST_F(BasicTransitionTests, transitionLowerLevelStateCorrectly){
    
    state_machine.processEvent(e1);
    EXPECT_TRUE(state_machine.is(houdini::state<IS21>, houdini::state<S2>))<< "Current state is " << state_machine.currentState();
    EXPECT_EQ(state_machine.currentStateName(), "IS21") << "Current state is " << state_machine.currentState();
    state_machine.processEvent(ie1);
    EXPECT_TRUE(state_machine.is(houdini::state<IS22>, houdini::state<S2>)) 
    << "Current state is " << state_machine.currentState();
    EXPECT_EQ(state_machine.currentStateName(), "IS22") << "Current state is " << state_machine.currentState();

}

TEST_F(BasicTransitionTests, higherLevelTransitionOverridesLowerLevel){
    state_machine.processEvent(e1);
    state_machine.processEvent(ie1);
    state_machine.processEvent(ie2);
    EXPECT_TRUE(state_machine.is(houdini::state<IS23>, houdini::state<S2>));
    EXPECT_EQ(state_machine.currentStateName(), "IS23");
    state_machine.processEvent(e3); 
    EXPECT_TRUE(state_machine.is(houdini::state<IS31>, houdini::state<S3>)) 
    << "A higher level transition should override the lower level transition\n"
    << "Current state is: " << state_machine.currentState();
    EXPECT_EQ(state_machine.currentStateName(), "IS31");
}

TEST_F(BasicTransitionTests, transitionHigherLevelStateCorrectly){
    state_machine.processEvent(e1);
    state_machine.processEvent(e3);
    ASSERT_TRUE(state_machine.is(houdini::state<IS31>, houdini::state<S3>)) 
    << "This should succeed unless the previous test failed. Otherwise, there is likely some nondetermistic behavior happening.\n"
    << "Current state is: " << state_machine.currentState()
    << "\nAbort test if this fails as the other checks are guaranteed to fail otherwise.";
    EXPECT_EQ(state_machine.currentStateName(), "IS31");
    state_machine.processEvent(ie1);
    EXPECT_TRUE(state_machine.is(houdini::state<IS32>, houdini::state<S3>));
    EXPECT_EQ(state_machine.currentStateName(), "IS32");
    state_machine.processEvent(e2);
    EXPECT_TRUE(state_machine.is(houdini::state<S4>));
    EXPECT_EQ(state_machine.currentStateName(), "S4");

}

TEST_F(BasicTransitionTests, repeatSameTransitionSuccessfully){
    state_machine.processEvent(e1);
    state_machine.processEvent(e2);
    EXPECT_TRUE(state_machine.is(houdini::state<IS31>, houdini::state<S3>));
    EXPECT_EQ(state_machine.currentStateName(), "IS31");
    state_machine.processEvent(e3);
    EXPECT_TRUE(state_machine.is(houdini::state<IS33>, houdini::state<S3>));
    EXPECT_EQ(state_machine.currentStateName(), "IS33");
    state_machine.processEvent(e3);
    EXPECT_TRUE(state_machine.is(houdini::state<IS31>, houdini::state<S3>));
    EXPECT_EQ(state_machine.currentStateName(), "IS31");
    state_machine.processEvent(e3);
    EXPECT_TRUE(state_machine.is(houdini::state<IS33>, houdini::state<S3>));
    EXPECT_EQ(state_machine.currentStateName(), "IS33");
    state_machine.processEvent(e3);
    EXPECT_TRUE(state_machine.is(houdini::state<IS31>, houdini::state<S3>));
    EXPECT_EQ(state_machine.currentStateName(), "IS31");
    state_machine.processEvent(e3);
    EXPECT_TRUE(state_machine.is(houdini::state<IS33>, houdini::state<S3>));
    EXPECT_EQ(state_machine.currentStateName(), "IS33");
    state_machine.processEvent(e3);
    EXPECT_TRUE(state_machine.is(houdini::state<IS31>, houdini::state<S3>));
    EXPECT_EQ(state_machine.currentStateName(), "IS31");
}

TEST_F(BasicTransitionTests, shouldTransitionToSameState){
    using namespace houdini;
    state_machine.processEvent(e1); //S2
    state_machine.processEvent(e2); //S3
    state_machine.processEvent(e2); //S4
    state_machine.processEvent(e4); //S4
    EXPECT_EQ(state_machine.currentStateName(), "S4");
    EXPECT_TRUE(state_machine.is(state<S4>));

}

TEST_F(BasicTransitionTests, shouldTransitionToSameNestedState){
    using namespace houdini;
    state_machine.processEvent(e1); //S2
    state_machine.processEvent(e4);
    EXPECT_EQ(state_machine.currentStateName(), "IS21");
    EXPECT_TRUE(state_machine.is(state<IS21>, state<S2>));
}

TEST_F(BasicTransitionTests, reentryToSameMultilevelStateHasSameInitialState){
    state_machine.processEvent(e1);
    state_machine.processEvent(ie1);
    EXPECT_TRUE(state_machine.is(houdini::state<IS22>, houdini::state<S2>));
    EXPECT_EQ(state_machine.currentStateName(), "IS22");
    state_machine.processEvent(e3);
    EXPECT_TRUE(state_machine.is(houdini::state<IS31>, houdini::state<S3>));
    EXPECT_EQ(state_machine.currentStateName(), "IS31");
    state_machine.processEvent(e1);
    EXPECT_TRUE(state_machine.is(houdini::state<IS21>, houdini::state<S2>))
        << "Should return to S2's initial state";
    EXPECT_EQ(state_machine.currentStateName(), "IS21");    
}

TEST_F(BasicTransitionTests, guardsAreFunctional){
    state_machine.processEvent(e3);
    EXPECT_TRUE(state_machine.is(houdini::state<S1>)) << "Guard should always fail";
    EXPECT_EQ(state_machine.currentStateName(), "S1");
    state_machine.processEvent(e4);
    EXPECT_TRUE(state_machine.is(houdini::state<S4>)) << "Guard should never fail";
    EXPECT_EQ(state_machine.currentStateName(), "S4");
    
}