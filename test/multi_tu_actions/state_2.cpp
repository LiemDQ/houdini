#include "state_2.hpp"

void State2::onEntry(TestContext& context, Broker&){
    context.l2 = context.l2*2;
}

void State2::onExit(TestContext& context, Broker&){
    context.l2 = context.l2/2;
}