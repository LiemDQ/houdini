#include "estate/actor/actor.hpp"
#include "test_actor_sm.hpp"
#include "gtest/gtest.h"

using namespace estate;
class BasicActorTests : public ::testing::Test {
    protected:
        estate::act::Actor<Events, Root, act::BaseContext, brokers::BaseBroker> actor;
};

TEST_F(BasicActorTests, shouldInitialize){
    
}