#include "basic_sm.hpp"
#include "estate/actor/context.hpp"
#include "estate/sm/backend/algorithms.hpp"
#include "estate/sm/backend/collect.hpp"
#include "estate/sm/backend/collect_initial_states.hpp"
#include "estate/sm/backend/flatten.hpp"
#include "estate/sm/backend/index.hpp"
#include "estate/sm/backend/resolve_state.hpp"
#include "estate/util/type_name.hpp"
#include <gtest/gtest.h>

class BasicTransitionTests : public ::testing::Test {
    protected:
        estate::act::BaseContext context;
        estate::brokers::BaseBroker broker;
        estate::SM<Root, Events> state_machine{context, broker};

};


TEST_F(BasicTransitionTests, stateMachineInitializesCorrectly){
    using ParentList = estate::sm::detail::TypeList<estate::sm::TState<Root>>;
    //auto ists = estate::sm::makeInitialStateMap(state_machine.root_state);

    EXPECT_EQ(1, estate::sm::getCombinedStateIndex(estate::sm::getCombinedStateTypeIDs(state_machine.root_state), 
        ParentList{}, estate::state<S1>)) << "Combined state table is correctly formulated?";

    EXPECT_EQ(1, state_machine.currentState()) << "Initial state index is correct?";
    EXPECT_EQ(state_machine.currentStateName(), "S1") << "Initial state name is initialized correctly?";
    EXPECT_TRUE(state_machine.is(estate::state<S1>));// << estate::util::type_name(estate::sm::getCombinedStateTypeIDs(state_machine.root_state));    
    
}

TEST_F(BasicTransitionTests, transitionToSameLevelState){
    EXPECT_TRUE(state_machine.is(estate::state<S1>));
    state_machine.processEvent(e4);
    EXPECT_EQ(state_machine.currentStateName(), "S4");
    EXPECT_TRUE(state_machine.is(estate::state<S4>));
        
}

TEST_F(BasicTransitionTests, transitionLowerLevelStateCorrectly){
    
    state_machine.processEvent(e1);
    EXPECT_TRUE(state_machine.is(estate::state<IS21>, estate::state<S2>))<< "Current state is " << state_machine.currentState();
    EXPECT_EQ(state_machine.currentStateName(), "IS21") << "Current state is " << state_machine.currentState();
    state_machine.processEvent(ie1);
    EXPECT_TRUE(state_machine.is(estate::state<IS22>, estate::state<S2>)) 
    << "Current state is " << state_machine.currentState();
    EXPECT_EQ(state_machine.currentStateName(), "IS22") << "Current state is " << state_machine.currentState();

}

TEST_F(BasicTransitionTests, higherLevelTransitionOverridesLowerLevel){
    state_machine.processEvent(e1);
    state_machine.processEvent(ie1);
    state_machine.processEvent(ie2);
    EXPECT_TRUE(state_machine.is(estate::state<IS23>, estate::state<S2>));
    EXPECT_EQ(state_machine.currentStateName(), "IS23");
    state_machine.processEvent(e3); 
    EXPECT_TRUE(state_machine.is(estate::state<IS31>, estate::state<S3>)) 
    << "A higher level transition should override the lower level transition\n"
    << "Current state is: " << state_machine.currentState();
    EXPECT_EQ(state_machine.currentStateName(), "IS31");
}

TEST_F(BasicTransitionTests, transitionHigherLevelStateCorrectly){
    state_machine.processEvent(e1);
    state_machine.processEvent(e3);
    ASSERT_TRUE(state_machine.is(estate::state<IS31>, estate::state<S3>)) 
    << "This should succeed unless the previous test failed. Otherwise, there is likely some nondetermistic behavior happening.\n"
    << "Current state is: " << state_machine.currentState()
    << "\nAbort test if this fails as the other checks are guaranteed to fail otherwise.";
    EXPECT_EQ(state_machine.currentStateName(), "IS31");
    state_machine.processEvent(ie1);
    EXPECT_TRUE(state_machine.is(estate::state<IS32>, estate::state<S3>));
    EXPECT_EQ(state_machine.currentStateName(), "IS32");
    state_machine.processEvent(e2);
    EXPECT_TRUE(state_machine.is(estate::state<S4>));
    EXPECT_EQ(state_machine.currentStateName(), "S4");

}

TEST_F(BasicTransitionTests, repeatSameTransitionSuccessfully){
    state_machine.processEvent(e1);
    state_machine.processEvent(e2);
    EXPECT_TRUE(state_machine.is(estate::state<IS31>, estate::state<S3>));
    EXPECT_EQ(state_machine.currentStateName(), "IS31");
    state_machine.processEvent(e3);
    EXPECT_TRUE(state_machine.is(estate::state<IS33>, estate::state<S3>));
    EXPECT_EQ(state_machine.currentStateName(), "IS33");
    state_machine.processEvent(e3);
    EXPECT_TRUE(state_machine.is(estate::state<IS31>, estate::state<S3>));
    EXPECT_EQ(state_machine.currentStateName(), "IS31");
    state_machine.processEvent(e3);
    EXPECT_TRUE(state_machine.is(estate::state<IS33>, estate::state<S3>));
    EXPECT_EQ(state_machine.currentStateName(), "IS33");
    state_machine.processEvent(e3);
    EXPECT_TRUE(state_machine.is(estate::state<IS31>, estate::state<S3>));
    EXPECT_EQ(state_machine.currentStateName(), "IS31");
    state_machine.processEvent(e3);
    EXPECT_TRUE(state_machine.is(estate::state<IS33>, estate::state<S3>));
    EXPECT_EQ(state_machine.currentStateName(), "IS33");
    state_machine.processEvent(e3);
    EXPECT_TRUE(state_machine.is(estate::state<IS31>, estate::state<S3>));
    EXPECT_EQ(state_machine.currentStateName(), "IS31");
}

TEST_F(BasicTransitionTests, shouldTransitionToSameState){
    using namespace estate;
    state_machine.processEvent(e1); //S2
    state_machine.processEvent(e2); //S3
    state_machine.processEvent(e2); //S4
    state_machine.processEvent(e4); //S4
    EXPECT_EQ(state_machine.currentStateName(), "S4");
    EXPECT_TRUE(state_machine.is(state<S4>));

}

TEST_F(BasicTransitionTests, shouldTransitionToSameNestedState){
    using namespace estate;
    state_machine.processEvent(e1); //S2
    state_machine.processEvent(e4);
    EXPECT_EQ(state_machine.currentStateName(), "IS21");
    EXPECT_TRUE(state_machine.is(state<IS21>, state<S2>));
}

TEST_F(BasicTransitionTests, reentryToSameMultilevelStateHasSameInitialState){
    state_machine.processEvent(e1);
    state_machine.processEvent(ie1);
    EXPECT_TRUE(state_machine.is(estate::state<IS22>, estate::state<S2>));
    EXPECT_EQ(state_machine.currentStateName(), "IS22");
    state_machine.processEvent(e3);
    EXPECT_TRUE(state_machine.is(estate::state<IS31>, estate::state<S3>));
    EXPECT_EQ(state_machine.currentStateName(), "IS31");
    state_machine.processEvent(e1);
    EXPECT_TRUE(state_machine.is(estate::state<IS21>, estate::state<S2>))
        << "Should return to S2's initial state";
    EXPECT_EQ(state_machine.currentStateName(), "IS21");    
}

TEST_F(BasicTransitionTests, guardsAreFunctional){
    state_machine.processEvent(e3);
    EXPECT_TRUE(state_machine.is(estate::state<S1>)) << "Guard should always fail";
    EXPECT_EQ(state_machine.currentStateName(), "S1");
    state_machine.processEvent(e4);
    EXPECT_TRUE(state_machine.is(estate::state<S4>)) << "Guard should never fail";
    EXPECT_EQ(state_machine.currentStateName(), "S4");
    
}