#include "houdini/sm/backend/collect_initial_states.hpp"
#include "houdini/sm/backend/pseudo_states.hpp"
#include "houdini/sm/backend/resolve_state.hpp"
#include "houdini/sm/backend/state.hpp"
#include "houdini/sm/backend/transition.hpp"
#include "houdini/sm/backend/type_list.hpp"
#include "houdini/sm/sm.hpp"
#include "houdini/sm/frontend/transition_dsl.hpp"

#include <gtest/gtest.h>

using namespace ::testing;

// states
struct S1 : houdini::State<>{};
struct S2 : houdini::State<>{};
struct S3 : houdini::State<>{};
struct S4 : houdini::State<>{};

// events
enum TestEvents {
	e1
};

JANUS_CREATE_EVENT(TestEvents, event);

struct S : houdini::State<> {
	constexpr auto on_entry() {
		return [](auto counter){ (*counter)++; };
	}

	constexpr auto on_exit(){
		return [](auto counter){(*counter)++; };
	}
};

struct SubState : houdini::State<> {
	static constexpr auto make_transition_table(){
		//clang-format off
		return houdini::transition_table(
			*houdini::state<S3> + event<e1> = houdini::state<S4>
		);
		//clang-format on
	}
};

struct MainState : houdini::State<> {
	static constexpr auto make_transition_table() {
		//clang-format off
		return houdini::transition_table(
			*houdini::state<S1> + event<e1> = houdini::state<S2>,
			 houdini::state<S2> + event<e1> = houdini::state<SubState>
		);
		//clang-format on
	}
};

TEST(ResolveHistoryTests, should_resolve_history_state){
	constexpr auto historyTransition = houdini::sm::detail::extended_transition(
		0, houdini::sm::detail::Transition<int, 2, int, int, houdini::sm::THistory<S>>{});

	constexpr auto noHistoryTransition = houdini::sm::detail::extended_transition(
		0, houdini::sm::detail::Transition<int, 2, int, int, houdini::sm::TState<S>>{});
	
	ASSERT_TRUE(houdini::sm::resolveHistory(historyTransition));
	ASSERT_FALSE(houdini::sm::resolveHistory(noHistoryTransition));
}

TEST(ResolveDestinationTests, should_resolve_destination){
	auto dst = houdini::state<S1>;
	auto transition = houdini::sm::detail::extended_transition(
		0, houdini::sm::detail::Transition<int,2,int,int,decltype(dst)>{});
	
	ASSERT_TRUE(dst == houdini::sm::resolveDst(transition));
}

TEST(ResolveDestinationTests, should_resolve_submachine_destination){
	// auto init = houdini::mp::mp_at_c<decltype(collectInitialStates(houdini::state<SubState>)), 0>{};
	// auto init2 = houdini::sm::collectInitialStates(houdini::state<MainState>);
	auto transition = houdini::sm::detail::extended_transition(
		houdini::sm::detail::TypeList<houdini::sm::TState<MainState>>{},
		houdini::sm::detail::Transition<int, 2, int, int, houdini::sm::TState<SubState>>());
	ASSERT_TRUE(houdini::state<S3> == houdini::sm::resolveDst(transition));
}

TEST(ResolveParentDestinationTests, should_resolve_destination_parent){
	auto dst = houdini::state<S1>;
	auto srcParent = houdini::state<S2>;
	auto transition = houdini::sm::detail::extended_transition(
		srcParent, houdini::sm::detail::Transition<int, 2, int, int, decltype(dst)>() 
	);
	ASSERT_TRUE(houdini::state<S2> == transition.parent());
}

TEST(ResolveParentDestinationTests, should_resolve_submachine_destination_parent){
	auto dst = houdini::state<SubState>;
	auto transition = houdini::sm::detail::extended_transition(
		0, houdini::sm::detail::Transition<int, 2, int, int, decltype(dst)>{});

	ASSERT_TRUE(dst == houdini::sm::resolveDstParent(transition));
}

TEST(ResolveSourceTests, should_resolve_source){
	auto src = houdini::state<S1>;
	auto transition = houdini::sm::detail::extended_transition(
		0, houdini::sm::detail::Transition<decltype(src), 2, int, int, int>());
	
	ASSERT_TRUE(src == houdini::sm::resolveSrc(transition));
}

TEST(ResolveSourceParentTests, should_resolve_source){
	auto srcParent = houdini::state<S2>;
	auto transition = houdini::sm::detail::extended_transition(
		srcParent, houdini::sm::detail::Transition<houdini::sm::TState<S1>, 2, int, int, int>());
	
	ASSERT_TRUE(srcParent == houdini::sm::resolveSrcParent(transition));
}