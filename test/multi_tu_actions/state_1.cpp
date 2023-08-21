#include "state_1.hpp"

void State1::onEntry(TestContext& context, Broker&){
    context.l1 = context.l1*context.l1;
}

void State1::onExit(TestContext& context, Broker&){
    context.l1 = context.l1*context.l1;
}