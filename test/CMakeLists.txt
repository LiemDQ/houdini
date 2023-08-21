cmake_minimum_required(VERSION 3.16)

project(houdini_tests CXX)

# dependencies

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY  https://github.com/google/googletest.git
    GIT_TAG         release-1.11.0
)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)
# flags

add_executable(
    smUnitTests
    sm/dispatch_table_tests.cpp
    sm/basic_transition_tests.cpp
    sm/direct_transition_tests.cpp
    sm/history_transition_tests.cpp
    sm/collect_tests.cpp
    )
    
add_executable(
    utilUnitTests
    utils/enum_util_tests.cpp
    utils/utility_function_tests.cpp
    )
    
add_executable(
    actorUnitTests
    actor/basic_actor_tests.cpp
    )
    
add_executable(
    actionsUnitTests
    actions/state_tests.cpp
    actions/action_tests.cpp
    )

add_executable(
    multiTUActionUnitTests
    multi_tu_actions/multi_tu_action_tests.cpp
    multi_tu_actions/state_1.cpp
    multi_tu_actions/state_2.cpp
)


foreach(name IN ITEMS sm util actor actions multiTUAction)
    target_link_libraries("${name}UnitTests" PUBLIC houdini_options houdini_warnings)
    target_link_libraries("${name}UnitTests" PUBLIC houdini GTest::gtest_main)
    gtest_discover_tests("${name}UnitTests" TEST_PREFIX "${name}.")
    
endforeach()

