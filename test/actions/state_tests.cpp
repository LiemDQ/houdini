#include "estate/actor/context.hpp"
#include "estate/brokers/message_broker.hpp"
#include "estate/estate.hpp"

#include "gtest/gtest.h"

struct Context : estate::act::BaseContext {
	int i = 0;
	int k = 0;
};

struct Broker : estate::brokers::BaseBroker {

};

using State = estate::State<Context, Broker>;

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