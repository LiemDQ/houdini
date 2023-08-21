#include "estate/util/utility_functions.hpp"
#include <gtest/gtest.h>

#include <array>
#include <algorithm>
#include <cstdlib>
#include <string_view>

TEST(TestUtilityFunctions, shouldFillArrayLikeStd){
    using namespace estate;
    constexpr std::size_t arr_length = 20;
    constexpr int fill_value = 5;

    std::array<int, arr_length> arr;
    
    constexpr auto filled_arr = util::fill_array(arr, fill_value);

    EXPECT_NE(arr, filled_arr);
    arr.fill(fill_value);
    EXPECT_EQ(arr, filled_arr);
    EXPECT_EQ(arr.max_size(), filled_arr.max_size());
}

TEST(TestUtilityFunctions, shouldProduceSameResultAsStdTransform){
    using namespace estate;
    auto squared = [](int x){return x*x; };

    constexpr std::size_t arr_length = 20;

    std::array<int, arr_length> arr;
    std::generate(arr.begin(), arr.end(), std::rand);
    
    std::array<int, arr_length> arr2(arr), arr3(arr);

    std::transform(arr.begin(), arr.end(), arr.begin(), squared);
    util::transform(arr2.begin(), arr2.end(), arr2.begin(), squared);

    EXPECT_NE(arr, arr3);

    EXPECT_EQ(arr, arr2);
}