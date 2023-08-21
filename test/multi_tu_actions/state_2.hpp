#pragma once
#include "houdini/houdini.hpp"
#include "action_context.hpp"
class State2 : public houdini::State<TestContext, Broker>{
    void onEntry(TestContext& context, Broker&) override;
    void onExit(TestContext& context, Broker&) override;
}; 