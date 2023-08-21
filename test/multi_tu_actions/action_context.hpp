#pragma once
#include "houdini/houdini.hpp"

struct TestContext : public houdini::Context<TestContext> {
	int l1 = 2;
	int l2 = 1;
	int l3 = 0;
};

enum Events : houdini::JEvent {
	e1,
	e2,
	ie1,
	ie2,
	iie1,
	iie2
};

JANUS_CREATE_EVENT(Events, event);

using Broker = houdini::brokers::MessageBroker<Events>;