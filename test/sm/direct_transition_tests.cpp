#include "houdini/brokers/message_broker.hpp"
#include "houdini/sm/backend/event.hpp"
#include "houdini/sm/backend/flatten.hpp"
#include "houdini/sm/backend/resolve_state.hpp"
#include "houdini/sm/backend/traits.hpp"
#include "houdini/sm/sm.hpp"
#include "houdini/util/types.hpp"
#include <gtest/gtest.h>

enum Events : houdini::JEvent {
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

JANUS_CREATE_EVENT(Events, event);

struct Inner1 : houdini::State<> {};

struct II1 : houdini::State<>{};
struct II2 : houdini::State<>{};


struct Inner2 : houdini::State<> {
    static constexpr auto make_transition_table(){
        //clang-format off
        using namespace houdini;
        return houdini::transition_table(
            *state<II1> + event<iie1> = state<II2>,
            state<II2> +  event<iie2> = state<II1>
        );
        //clang-format on
    }
};

struct Inner3 : houdini::State<> {
    static constexpr auto make_transition_table(){
        //clang-format off
        using namespace houdini;
        return houdini::transition_table(
            *state<II1> + event<iie1> = state<II2>,
            state<II2> +  event<iie2> = state<II1>
        );
        //clang-format on
    }
};

struct Inner5 : houdini::State<> {
    static constexpr auto make_transition_table(){
        //clang-format off
        using namespace houdini;
        return houdini::transition_table(
            *state<II1> + event<iie1> = state<II2>,
            state<II2> +  event<iie2> = state<II1>
        );
        //clang-format on
    }
};

struct Inner4 : houdini::State<> {};

struct S1 : houdini::State<> {};

struct S2 : houdini::State<> {
    static constexpr auto make_transition_table(){
        //clang-format off
        using namespace houdini;
        return houdini::transition_table(
            *state<Inner1> + event<ie1> = state<Inner2>,
            state<Inner2> +  event<ie1> = state<Inner3>,
            state<Inner3> + event<ie1> = state<Inner4>,
            state<Inner4> + event<ie1> = state<Inner1>,
            state<Inner1> + event<ie2> = state<Inner4>,
            state<Inner2> +  event<ie2> = state<Inner1>,
            state<Inner3> + event<ie2> = state<Inner2>,
            state<Inner4> + event<ie2> = state<Inner3>

        );
        //clang-format on
    }
};


struct S3 : houdini::State<> {
    static constexpr auto make_transition_table(){
        //clang-format off
        using namespace houdini;
        return houdini::transition_table(
            *state<Inner1> + event<ie1> = state<Inner2>,
            state<Inner2> +  event<ie1> = state<Inner3>,
            state<Inner3> + event<ie1> = state<Inner4>,
            state<Inner4> + event<ie1> = state<Inner5>,
            state<Inner5> + event<ie1> = state<Inner1>,
            state<Inner1> + event<ie2> = state<Inner5>,
            state<Inner2> +  event<ie2> = state<Inner1>,
            state<Inner3> + event<ie2> = state<Inner2>,
            state<Inner4> + event<ie2> = state<Inner3>,
            state<Inner5> + event<ie2> = state<Inner4>
        );
    }
};

struct DRoot : houdini::State<> {
    static constexpr auto make_transition_table(){
        //clang-format off
        using namespace houdini;
        return houdini::transition_table(
            *state<S1> + event<e1> = state<S2>,
            state<S2> + event<e1> = state<S3>,
            state<S3> + event<e1> = state<S1>,
            state<S1> + event<e2> = state<S3>,
            state<S3> + event<e2> = state<S2>,
            state<S2> + event<e2> = state<S1>,
            state<S1> + event<e3> = direct<Inner2, S2>,
            state<S1> + event<e4> = direct<Inner4, S2>,
            state<S2> + event<e3> = direct<Inner3, S3>,
            state<S2> + event<e4> = direct<Inner4, S3>,
            direct<Inner4, S3> + event<ie3> = state<S1>,
            direct<Inner1, S2> + event<ie3> = direct<Inner4, S3>,
            direct<II2, Inner2, S2> + event<iie3> = state<S1>,
            direct<II2, Inner2, S2> + event<iie4> = state<S2>,
            direct<II1, Inner2, S2> + event<iie3> = direct<II2, Inner5, S3>
        );
        //clang-format on
    }    
};

class DirectTransitionTests : public ::testing::Test {
    protected:
        houdini::act::BaseContext context;
        houdini::brokers::BaseBroker broker;
        houdini::SM<DRoot, Events> state_machine{context, broker};
};

TEST_F(DirectTransitionTests, basicDirectTransitionCorrect){
    using namespace houdini;
    sm::flattenTransitionTable(state_machine.root_state);
    //auto lst = sm::getParentStateList(direct<II2, Inner4, S3>);
    //auto transition = houdini::sm::detail::ExtendedTransition<boost::mp11::mp_list<houdini::sm::TState<DRoot> >, houdini::sm::TState<houdini::sm::Direct<houdini::sm::TState<II2>, houdini::sm::TState<Inner2> > >, 10, houdini::sm::NoGuard, houdini::sm::NoAction, houdini::sm::TState<S1>, false>{};
    //auto dst = houdini::sm::resolveSrcParents(transition);
    ASSERT_TRUE(state_machine.is(state<S1>));
    state_machine.processEvent(e1);
    ASSERT_TRUE(state_machine.is(state<Inner1>, state<S2>));
    state_machine.processEvent(e2);
    state_machine.processEvent(e4);
    EXPECT_TRUE(state_machine.is(state<Inner4>, state<S2>));
    EXPECT_EQ(state_machine.currentStateName(), "Inner4");
}

TEST_F(DirectTransitionTests, transitionBetweenLowerLevelStatesCorrect){
    using namespace houdini;
    ASSERT_TRUE(state_machine.is(state<S1>));
    state_machine.processEvent(e1);
    ASSERT_TRUE(state_machine.is(state<Inner1>, state<S2>));
    state_machine.processEvent(ie3);
    EXPECT_TRUE(state_machine.is(state<Inner4>, state<S3>));
    EXPECT_EQ(state_machine.currentStateName(), "Inner4");
}

TEST_F(DirectTransitionTests, transitionFromLowerStateToHigherCorrect){
    using namespace houdini;
    ASSERT_TRUE(state_machine.is(state<S1>));
    state_machine.processEvent(e2);
    ASSERT_TRUE(state_machine.is(state<Inner1>, state<S3>));
    state_machine.processEvent(ie2);
    EXPECT_TRUE(state_machine.is(state<II1>, state<Inner5>, state<S3>));
    EXPECT_EQ(state_machine.currentStateName(), "II1") << "Should be in Inner5";
    state_machine.processEvent(ie2);
    state_machine.processEvent(ie3);
    EXPECT_TRUE(state_machine.is(state<S1>));
    EXPECT_EQ(state_machine.currentStateName(), "S1");
}

TEST_F(DirectTransitionTests, shouldtransition2LevelsUp){
    using namespace houdini;
    ASSERT_TRUE(state_machine.is(state<S1>));
    state_machine.processEvent(e1);
    ASSERT_TRUE(state_machine.is(state<Inner1>, state<S2>));
    state_machine.processEvent(ie1);
    EXPECT_TRUE(state_machine.is(state<II1>, state<Inner2>, state<S2>));
    EXPECT_EQ(state_machine.currentStateName(), "II1");
    state_machine.processEvent(iie1);
    EXPECT_TRUE(state_machine.is(state<II2>, state<Inner2>, state<S2>));
    state_machine.processEvent(iie3);
    EXPECT_TRUE(state_machine.is(state<S1>));
    EXPECT_EQ(state_machine.currentStateName(), "S1");
}

TEST_F(DirectTransitionTests, shouldReachCorrectInitialStateFromTransition2LevelsUp){
    using namespace houdini;
    ASSERT_TRUE(state_machine.is(state<S1>));
    state_machine.processEvent(e1);
    ASSERT_TRUE(state_machine.is(state<Inner1>, state<S2>));
    state_machine.processEvent(ie1);
    EXPECT_TRUE(state_machine.is(state<II1>, state<Inner2>, state<S2>));
    EXPECT_EQ(state_machine.currentStateName(), "II1");
    state_machine.processEvent(iie1);
    EXPECT_TRUE(state_machine.is(state<II2>, state<Inner2>, state<S2>))
    << "This is a standard transition and should not fail if previous tests have passed.";
    state_machine.processEvent(iie4);
    EXPECT_TRUE(state_machine.is(state<Inner1>, state<S2>)) 
    << "Transitioning to S2 should reset back to the initial state.";
    EXPECT_EQ(state_machine.currentStateName(), "Inner1");
}

TEST_F(DirectTransitionTests, shouldTransition2LevelsAcross){
    using namespace houdini;
    ASSERT_TRUE(state_machine.is(state<S1>));
    state_machine.processEvent(e1);
    ASSERT_TRUE(state_machine.is(state<Inner1>, state<S2>));
    state_machine.processEvent(ie1);
    EXPECT_TRUE(state_machine.is(state<II1>, state<Inner2>, state<S2>));
    EXPECT_EQ(state_machine.currentStateName(), "II1");
    state_machine.processEvent(iie3);
    EXPECT_TRUE(state_machine.is(state<II2>, state<Inner5>, state<S3>));
    EXPECT_EQ(state_machine.currentStateName(), "II2");
    state_machine.processEvent(ie1);
    EXPECT_TRUE(state_machine.is(state<Inner1>, state<S3>));
    state_machine.processEvent(e1);
    EXPECT_TRUE(state_machine.is(state<S1>)) << "After transitioning to Inner5, superstate should be S3.";
    EXPECT_EQ(state_machine.currentStateName(), "S1");

}

TEST_F(DirectTransitionTests, shouldTransitionFromUpperLevelState2LevelsUp){
    using namespace houdini;
    ASSERT_TRUE(state_machine.is(state<S1>));
    state_machine.processEvent(e1);
    ASSERT_TRUE(state_machine.is(state<Inner1>, state<S2>));
    state_machine.processEvent(ie1);
    EXPECT_TRUE(state_machine.is(state<II1>, state<Inner2>, state<S2>));
    EXPECT_EQ(state_machine.currentStateName(), "II1");
    state_machine.processEvent(e2);
    EXPECT_TRUE(state_machine.is(state<S1>)) << "We are still in state S2, so transitions from S2 should still be valid";
    EXPECT_EQ(state_machine.currentStateName(), "S1");
}