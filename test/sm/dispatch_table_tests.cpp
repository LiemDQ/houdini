#include "estate/sm/backend/collect_initial_states.hpp"
#include "estate/sm/backend/pseudo_states.hpp"
#include "estate/sm/backend/resolve_state.hpp"
#include "estate/sm/backend/state.hpp"
#include "estate/sm/backend/transition.hpp"
#include "estate/sm/backend/type_list.hpp"
#include "estate/sm/sm.hpp"
#include "estate/sm/frontend/transition_dsl.hpp"

#include <gtest/gtest.h>

using namespace ::testing;

// states
struct S1 : estate::State<>{};
struct S2 : estate::State<>{};
struct S3 : estate::State<>{};
struct S4 : estate::State<>{};

// events
enum TestEvents {
	e1
};

JANUS_CREATE_EVENT(TestEvents, event);

struct S : estate::State<> {
	constexpr auto on_entry() {
		return [](auto counter){ (*counter)++; };
	}

	constexpr auto on_exit(){
		return [](auto counter){(*counter)++; };
	}
};

struct SubState : estate::State<> {
	static constexpr auto make_transition_table(){
		//clang-format off
		return estate::transition_table(
			*estate::state<S3> + event<e1> = estate::state<S4>
		);
		//clang-format on
	}
};

struct MainState : estate::State<> {
	static constexpr auto make_transition_table() {
		//clang-format off
		return estate::transition_table(
			*estate::state<S1> + event<e1> = estate::state<S2>,
			 estate::state<S2> + event<e1> = estate::state<SubState>
		);
		//clang-format on
	}
};

TEST(ResolveHistoryTests, should_resolve_history_state){
	constexpr auto historyTransition = estate::sm::detail::extended_transition(
		0, estate::sm::detail::Transition<int, 2, int, int, estate::sm::THistory<S>>{});

	constexpr auto noHistoryTransition = estate::sm::detail::extended_transition(
		0, estate::sm::detail::Transition<int, 2, int, int, estate::sm::TState<S>>{});
	
	ASSERT_TRUE(estate::sm::resolveHistory(historyTransition));
	ASSERT_FALSE(estate::sm::resolveHistory(noHistoryTransition));
}

TEST(ResolveDestinationTests, should_resolve_destination){
	auto dst = estate::state<S1>;
	auto transition = estate::sm::detail::extended_transition(
		0, estate::sm::detail::Transition<int,2,int,int,decltype(dst)>{});
	
	ASSERT_TRUE(dst == estate::sm::resolveDst(transition));
}

TEST(ResolveDestinationTests, should_resolve_submachine_destination){
	// auto init = estate::mp::mp_at_c<decltype(collectInitialStates(estate::state<SubState>)), 0>{};
	// auto init2 = estate::sm::collectInitialStates(estate::state<MainState>);
	auto transition = estate::sm::detail::extended_transition(
		estate::sm::detail::TypeList<estate::sm::TState<MainState>>{},
		estate::sm::detail::Transition<int, 2, int, int, estate::sm::TState<SubState>>());
	ASSERT_TRUE(estate::state<S3> == estate::sm::resolveDst(transition));
}

TEST(ResolveParentDestinationTests, should_resolve_destination_parent){
	auto dst = estate::state<S1>;
	auto srcParent = estate::state<S2>;
	auto transition = estate::sm::detail::extended_transition(
		srcParent, estate::sm::detail::Transition<int, 2, int, int, decltype(dst)>() 
	);
	ASSERT_TRUE(estate::state<S2> == transition.parent());
}

TEST(ResolveParentDestinationTests, should_resolve_submachine_destination_parent){
	auto dst = estate::state<SubState>;
	auto transition = estate::sm::detail::extended_transition(
		0, estate::sm::detail::Transition<int, 2, int, int, decltype(dst)>{});

	ASSERT_TRUE(dst == estate::sm::resolveDstParent(transition));
}

TEST(ResolveSourceTests, should_resolve_source){
	auto src = estate::state<S1>;
	auto transition = estate::sm::detail::extended_transition(
		0, estate::sm::detail::Transition<decltype(src), 2, int, int, int>());
	
	ASSERT_TRUE(src == estate::sm::resolveSrc(transition));
}

TEST(ResolveSourceParentTests, should_resolve_source){
	auto srcParent = estate::state<S2>;
	auto transition = estate::sm::detail::extended_transition(
		srcParent, estate::sm::detail::Transition<estate::sm::TState<S1>, 2, int, int, int>());
	
	ASSERT_TRUE(srcParent == estate::sm::resolveSrcParent(transition));
}