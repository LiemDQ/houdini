#include "basic_sm.hpp"
#include "estate/sm/backend/event.hpp"
#include "estate/sm/backend/flatten.hpp"
#include "estate/sm/backend/collect.hpp"
#include "estate/sm/backend/remove_duplicates.hpp"
#include "estate/util/mp11.hpp"
#include "estate/util/enum_utils.hpp"
#include "estate/util/type_name.hpp"
#include <gtest/gtest.h>
#include <array>

class CollectionTests : public ::testing::Test {
    protected:
        estate::act::BaseContext context;
        estate::brokers::BaseBroker broker;
        estate::SM<Root, Events> state_machine{context, broker};
        static constexpr std::size_t num_transitions = 
                estate::mp::mp_size<decltype(S2::make_transition_table())>::value
            +   estate::mp::mp_size<decltype(S3::make_transition_table())>::value
            +   estate::mp::mp_size<decltype(Root::make_transition_table())>::value;

};

TEST_F(CollectionTests, transitionTableCorrectlyFlattened){
    auto transitions = estate::sm::removeDuplicates(estate::sm::flattenTransitionTable(state_machine.root_state));
    std::size_t n_transitions = estate::mp::mp_size<decltype(transitions)>::value;
    EXPECT_EQ(num_transitions, n_transitions); 
}

TEST_F(CollectionTests, dispatchMapIsCorrect){
    using Map = decltype(estate::sm::collectStatesWithParents(estate::state<Root>));
    //auto map = estate::sm::collectStatesWithParents(estate::state<Root>);
    //EXPECT_EQ(estate::util::type_name<Map>(), "") << expose underlying layout of map;
    EXPECT_EQ(estate::mp::mp_size<Map>::value, 11) 
    << "There should be 10 possible permutations of states + 1 root state.";
}

