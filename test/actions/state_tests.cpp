#include "houdini/actor/context.hpp"
#include "houdini/brokers/message_broker.hpp"
#include "houdini/houdini.hpp"

#include "gtest/gtest.h"

struct Context : houdini::act::BaseContext {
	int i = 0;
	int k = 0;
};

struct Broker : houdini::brokers::BaseBroker {

};

using State = houdini::State<Context, Broker>;

struct TestState : State {
	TestState() {
		std::cout << "Creating S1" << std::endl;
	}

	void onEntry(Context& context, Broker&) override {
		std::cout << "Entering S1" << std::endl;
		context.i++;
	}

	void onExit(Context& context, Broker&) override {
		std::cout << "Exiting S1" << std::endl;
		context.i++;
	}

};


TEST(StateTests, virtualFunctionsShouldOverride){
	Context context;
	Broker broker;

	State* s1 = new TestState;
	//s1->onExit(context,broker);
	s1->onExitImpl(context, broker);
	
	EXPECT_EQ(context.i, 1);
}