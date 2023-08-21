#include "estate/actor/context.hpp"
#include "estate/brokers/message_broker.hpp"
#include "estate/sm/backend/event.hpp"
#include "estate/sm/sm.hpp"
#include "gtest/gtest.h"
enum HEvents : estate::JEvent {
    e1,
    e2,
    e3,
    e4,
    ie1,
    ie2,
    ie3,
    ie4,
    iie1,
    iie2,
    iie3,
    iie4
};

JANUS_CREATE_EVENT(HEvents, event);

struct II1: estate::State<> {};
struct II2: estate::State<> {};

struct Inner1: estate::State<> {};

struct Inner2: estate::State<>  {
    static constexpr auto make_transition_table(){
        //clang-format off
        using namespace estate;
        return estate::transition_table(
            *state<II1> + event<iie1> = state<II2>,
            state<II2> + event<iie1> = state<II1>
        );
        //clang-format on
    }
};

struct Inner3: estate::State<> {};

struct S1: estate::State<>  {};
struct S2: estate::State<>  {
    static constexpr auto make_transition_table(){
        using namespace estate;
        return estate::transition_table(
            *state<Inner1> + event<ie1> = state<Inner2>,
             state<Inner2> + event<ie1> = state<Inner3>,
             state<Inner3> + event<ie1> = state<Inner1>,
             state<Inner1> + event<ie2> = state<Inner3>,
             state<Inner2> + event<ie2> = state<Inner1>,
             state<Inner3> + event<ie2> = state<Inner2>,
             state<Inner1> + event<ie3> = history<Inner2>
        );
    }
};

struct S3 : estate::State<> {
    static constexpr auto make_transition_table(){
        using namespace estate;
        return estate::transition_table(
            *state<Inner1> + event<ie1> = state<Inner2>,
             state<Inner2> + event<ie1> = state<Inner3>,
             state<Inner3> + event<ie1> = state<Inner1>,
             state<Inner1> + event<ie2> = state<Inner3>,
             state<Inner2> + event<ie2> = state<Inner1>,
             state<Inner3> + event<ie2> = state<Inner2>
        );
    }
};

struct HRoot: estate::State<>  {
    static constexpr auto make_transition_table(){
        using namespace estate;
        return estate::transition_table(
            *state<S1> + event<e1> = state<S2>,
             state<S2> + event<e1> = state<S3>,
             state<S3> + event<e1> = state<S1>,
             state<S1> + event<e2> = history<S2>,
             state<S1> + event<e3>  = history<Inner2, S2>,
             direct<Inner3, S3> + event<e4> = history<S2>,
             direct<Inner1, S3> + event<e4> = history<Inner2, S2>
        );
    }
};

class HistoryTransitionTests : public ::testing::Test {
    protected:
        estate::act::BaseContext context;
        estate::brokers::BaseBroker broker;
        estate::SM<HRoot, HEvents> state_machine{context, broker};
};

TEST_F(HistoryTransitionTests, shouldReturnToHistoricalState){
    using namespace estate;
    ASSERT_TRUE(state_machine.is(state<S1>));
    state_machine.processEvent(e1); //S2, Inner1
    state_machine.processEvent(ie1); //S2, Inner2, II1
    EXPECT_TRUE(state_machine.is(state<II1>, state<Inner2>, state<S2>));
    EXPECT_EQ(state_machine.currentStateName(), "II1");
    state_machine.processEvent(ie1); //S2, Inner3
    EXPECT_TRUE(state_machine.is(state<Inner3>, state<S2>));
    state_machine.processEvent(e1); //S3
    state_machine.processEvent(e1); //S1
    EXPECT_TRUE(state_machine.is(state<S1>));
    state_machine.processEvent(e2); //S3
    EXPECT_TRUE(state_machine.is(state<Inner3>, state<S2>)) <<
    "Should return to most recently visited state";
    EXPECT_EQ(state_machine.currentStateName(), "Inner3");
}

TEST_F(HistoryTransitionTests, shouldReturnToLowerHistoricalState){
    using namespace estate;
    ASSERT_TRUE(state_machine.is(state<S1>));
    state_machine.processEvent(e1); //S2, Inner1
    state_machine.processEvent(ie1); //S2, Inner2, II1
    EXPECT_TRUE(state_machine.is(state<II1>, state<Inner2>, state<S2>));
    EXPECT_EQ(state_machine.currentStateName(), "II1");
    state_machine.processEvent(iie1); //S2, Inner2, II2
    EXPECT_TRUE(state_machine.is(state<II2>, state<Inner2>, state<S2>));
    state_machine.processEvent(e1); //S3
    state_machine.processEvent(e1); //S1
    state_machine.processEvent(e2); //S2, Inner2, II2
    EXPECT_TRUE(state_machine.is(state<II2>, state<Inner2>, state<S2>))
    << "State of lower nested states should also be preserved";
    EXPECT_EQ(state_machine.currentStateName(), "II2");

}

TEST_F(HistoryTransitionTests, shouldReturnToNestedHistoricalState){
    using namespace estate;
    ASSERT_TRUE(state_machine.is(state<S1>));
    state_machine.processEvent(e1); //S2, Inner1
    state_machine.processEvent(ie1); //S2, Inner2, II1
    EXPECT_TRUE(state_machine.is(state<II1>, state<Inner2>, state<S2>));
    state_machine.processEvent(iie1); //S2, Inner2, II2
    EXPECT_EQ(state_machine.currentStateName(), "II2");
    EXPECT_TRUE(state_machine.is(state<II2>, state<Inner2>, state<S2>));
    state_machine.processEvent(e1); //S3
    state_machine.processEvent(e1); //S1
    state_machine.processEvent(e3);
    EXPECT_EQ(state_machine.currentStateName(), "II2");
    EXPECT_TRUE(state_machine.is(state<II2>, state<Inner2>, state<S2>));
}

TEST_F(HistoryTransitionTests, shouldDirectTransitionToHistoricalState){
    using namespace estate;
    ASSERT_TRUE(state_machine.is(state<S1>));
    state_machine.processEvent(e1); //S2, Inner1
    state_machine.processEvent(ie1); //S2, Inner2, II1
    EXPECT_TRUE(state_machine.is(state<II1>, state<Inner2>, state<S2>));
    state_machine.processEvent(ie1); //S2, Inner3
    EXPECT_TRUE(state_machine.is(state<Inner3>, state<S2>));
    EXPECT_EQ(state_machine.currentStateName(), "Inner3");
    state_machine.processEvent(e1); //S3, Inner1
    state_machine.processEvent(ie2); //S3, Inner3
    state_machine.processEvent(e4); //S2, Inner2, II2
    EXPECT_TRUE(state_machine.is(state<Inner3>, state<S2>))
    << "Direct transitions should work in combination with historical transitions";
    EXPECT_EQ(state_machine.currentStateName(), "Inner3");
}

TEST_F(HistoryTransitionTests, shouldDirectTransitionToNestedHistoricalState){
    using namespace estate;
    ASSERT_TRUE(state_machine.is(state<S1>));
    state_machine.processEvent(e1); //S2, Inner1
    state_machine.processEvent(ie1); //S2, Inner2, II1
    EXPECT_TRUE(state_machine.is(state<II1>, state<Inner2>, state<S2>));
    state_machine.processEvent(iie1); //S2, Inner2, II2
    EXPECT_EQ(state_machine.currentStateName(), "II2");
    EXPECT_TRUE(state_machine.is(state<II2>, state<Inner2>, state<S2>));
    state_machine.processEvent(e1); //S3
    state_machine.processEvent(e4); //S2, Inner2, II2
    EXPECT_TRUE(state_machine.is(state<II2>, state<Inner2>, state<S2>)) <<
    "This history state transition occurs in a specific state 2 levels down" 
    " and should go to the specific history of state Inner2.";
    EXPECT_EQ(state_machine.currentStateName(), "II2");    
}

TEST_F(HistoryTransitionTests, shouldDirectTransitionToSpecifiedNestedHistoricalState){
    using namespace estate;
    ASSERT_TRUE(state_machine.is(state<S1>));
    state_machine.processEvent(e1); //S2, Inner1
    state_machine.processEvent(ie1); //S2, Inner2, II1
    EXPECT_TRUE(state_machine.is(state<II1>, state<Inner2>, state<S2>));
    state_machine.processEvent(iie1);
    EXPECT_TRUE(state_machine.is(state<II2>, state<Inner2>, state<S2>));
    state_machine.processEvent(ie1); //S2, Inner3
    EXPECT_TRUE(state_machine.is(state<Inner3>, state<S2>));
    EXPECT_EQ(state_machine.currentStateName(), "Inner3");
    state_machine.processEvent(e1); //S3, Inner1
    state_machine.processEvent(e4); //S2, Inner2, II2
    EXPECT_TRUE(state_machine.is(state<II2>, state<Inner2>, state<S2>)) <<
    "Although S2 was most recently in Inner3, we want to specifically transition to the most recent state in Inner2.";
    EXPECT_EQ(state_machine.currentStateName(), "II2");    
}