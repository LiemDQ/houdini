#include "basic_sm.hpp"
#include "houdini/sm/backend/event.hpp"
#include "houdini/sm/backend/flatten.hpp"
#include "houdini/sm/backend/collect.hpp"
#include "houdini/sm/backend/remove_duplicates.hpp"
#include "houdini/util/mp11.hpp"
#include "houdini/util/enum_utils.hpp"
#include "houdini/util/type_name.hpp"
#include <gtest/gtest.h>
#include <array>

class CollectionTests : public ::testing::Test {
    protected:
        houdini::act::BaseContext context;
        houdini::brokers::BaseBroker broker;
        houdini::SM<Root, Events> state_machine{context, broker};
        static constexpr std::size_t num_transitions = 
                houdini::mp::mp_size<decltype(S2::make_transition_table())>::value
            +   houdini::mp::mp_size<decltype(S3::make_transition_table())>::value
            +   houdini::mp::mp_size<decltype(Root::make_transition_table())>::value;

};

TEST_F(CollectionTests, transitionTableCorrectlyFlattened){
    auto transitions = houdini::sm::removeDuplicates(houdini::sm::flattenTransitionTable(state_machine.root_state));
    std::size_t n_transitions = houdini::mp::mp_size<decltype(transitions)>::value;
    EXPECT_EQ(num_transitions, n_transitions); 
}

TEST_F(CollectionTests, dispatchMapIsCorrect){
    using Map = decltype(houdini::sm::collectStatesWithParents(houdini::state<Root>));
    //auto map = houdini::sm::collectStatesWithParents(houdini::state<Root>);
    //EXPECT_EQ(houdini::util::type_name<Map>(), "") << expose underlying layout of map;
    EXPECT_EQ(houdini::mp::mp_size<Map>::value, 11) 
    << "There should be 10 possible permutations of states + 1 root state.";
}

