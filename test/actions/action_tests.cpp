#include "houdini/houdini.hpp"
#include "houdini/actor/actor_inc.hpp"
#include "houdini/brokers/message_broker.hpp"

#include "action_context.hpp"
#include "gtest/gtest.h"

#include <iostream>



struct II1 : public houdini::State<TestContext, Broker> {
	void onEntry(TestContext& context, Broker&) override {
		context.l3++;
	}

	void onExit(TestContext& context, Broker&) override {
		context.l3--;
	}
	
};

struct II2 : public houdini::State<TestContext, Broker> {

};

struct Inner1 : public houdini::State<TestContext, Broker> {
	void onEntry(TestContext& context, Broker&) override {
		context.l2++;
	}

	void onExit(TestContext& context, Broker&) override {
		context.l2--;
	}

};

struct Inner2 : houdini::State<TestContext, Broker> {

	static constexpr auto make_transition_table(){
		//clang-format off
		using namespace houdini;
		return transition_table(
			*state<II1> + event<iie1> = state<II2>,
			 state<II2> + event<iie1> = state<II1>
		);
		//clang-format on
	}
};

struct S1 : houdini::State<TestContext, Broker> {
};

struct S2 : houdini::State<TestContext, Broker> {
	void onEntry(TestContext& context, Broker&) override {
		std::cout << "Entering S2" << std::endl;
		std::cout << "Context val: " << context.l1 << std::endl;
		context.l1++;
		std::cout << "Context val: " << context.l1 << std::endl;
	}

	void onExit(TestContext& context, Broker&) override {
		context.l1--;
	}

};

struct S3 : houdini::State<TestContext, Broker> {
	
	void onEntry(TestContext& context, Broker&) override {
		context.l1++;
	}

	void onExit(TestContext& context, Broker&) override {
		context.l1--;
	}

	static constexpr auto make_transition_table(){
		//clang-format on
		using namespace houdini;
		return transition_table(
			*state<Inner1> + event<ie1> = state<Inner2>,
			 state<Inner2> + event<ie1> = state<Inner1>
		);
		//clang-format off
	}
};

struct ARoot : houdini::State<TestContext, Broker> {
	static constexpr auto make_transition_table(){
		//clang-format on
		using namespace houdini;
		return transition_table(
			*state<S1> + event<e1> = state<S2>,
			 state<S2> + event<e1> = state<S3>,
			 state<S3> + event<e1> = state<S1>,
			 state<S1> + event<e2> = state<S3>,
			 state<S2> + event<e2> = state<S1>,
			 state<S3> + event<e2> = state<S2>
		);
		//clang-format off
	}
};

class TransitionActionTests : public ::testing::Test {
	protected:
		TransitionActionTests() : state_machine(context, broker) {}
		TestContext context;
		Broker broker;

		houdini::SM<ARoot, Events, TestContext, Broker> state_machine;
};

 
TEST_F(TransitionActionTests, basicTransitionShouldCallEntryAction){
	ASSERT_EQ(context.l1, 0);
	state_machine.processEvent(e1);
	EXPECT_EQ(state_machine.currentStateName(), "S2");
	EXPECT_TRUE(state_machine.is(houdini::state<S2>));
	EXPECT_EQ(context.l1, 1) << "S2 on entry should get called exactly once";
}

TEST_F(TransitionActionTests, basicTransitionShouldCallExitAction){
	ASSERT_EQ(context.l1, 0);
	state_machine.processEvent(e1);
	EXPECT_EQ(state_machine.currentStateName(), "S2");
	EXPECT_EQ(context.l1, 1);
	state_machine.processEvent(e2);
	EXPECT_EQ(context.l1, 0) << "Exit action should reset the l1 count back to 0";
}

TEST_F(TransitionActionTests, multiLevelTransitionShouldCallParentEntryAction){
	ASSERT_EQ(context.l1, 0);
	state_machine.processEvent(e1);
	EXPECT_EQ(state_machine.currentStateName(), "S2");
	EXPECT_EQ(context.l1, 1);
	EXPECT_EQ(context.l2, 0);
	state_machine.processEvent(e1); // Inner1, S3
	EXPECT_EQ(state_machine.currentStateName(), "Inner1");
	EXPECT_EQ(context.l1, 1) << "l1 is incremented by the parent state S3";
	EXPECT_EQ(context.l2, 1) << "l2 is incremented by the child state Inner1";
}

TEST_F(TransitionActionTests, multiLevelTransitionShouldCallParentExitAction){
	ASSERT_EQ(context.l1, 0);
	state_machine.processEvent(e1);
	EXPECT_EQ(state_machine.currentStateName(), "S2");
	EXPECT_EQ(context.l1, 1);
	EXPECT_EQ(context.l2, 0);
	state_machine.processEvent(e1); // Inner1, S3
	EXPECT_EQ(state_machine.currentStateName(), "Inner1");
	EXPECT_EQ(context.l1, 1) << "l1 is incremented by the parent state S3";
	EXPECT_EQ(context.l2, 1) << "l2 is incremented by the child state Inner1";
}