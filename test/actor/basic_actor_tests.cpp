#include "houdini/actor/actor.hpp"
#include "test_actor_sm.hpp"
#include "gtest/gtest.h"

using namespace houdini;
class BasicActorTests : public ::testing::Test {
    protected:
        houdini::act::Actor<Events, Root, act::BaseContext, brokers::BaseBroker> actor;
};

TEST_F(BasicActorTests, shouldInitialize){
    
}