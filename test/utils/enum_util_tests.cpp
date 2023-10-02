#include "houdini/util/enum_utils.hpp"
#include "houdini/util/types.hpp"

#include <gtest/gtest.h>
#include <algorithm>
#include <string_view>
#include <array>

class EnumUtilTests : public ::testing::Test {
	protected:
	
	enum TestEnum : houdini::JEvent {
		v0,
		v1,
		v2,
		v3,
		v4
	};

	enum TestEnum2 {
		t0 = 10,
		t1,
		t2,
		t3,
		t4 = 255
	};

	enum class ScopedEnum : houdini::JEvent {
		s0,
		s1 = 30,
		s2 = 2,
		s3,
		s4
	};

	enum class ScopedEnum2 : houdini::JEvent {
		s0,
		s1,
		s2,
		s3 = 233,
		s4
	};

};

using namespace houdini;
TEST_F(EnumUtilTests, enumMaxValueIsCorrect){	
	EXPECT_EQ(util::enum_max_value<TestEnum>(), 4);
	EXPECT_EQ(util::enum_max_value<ScopedEnum>(), 30);
	EXPECT_EQ(util::enum_max_value<TestEnum2>(), 255);
	EXPECT_EQ(util::enum_max_value<ScopedEnum2>(), 234);
}

TEST_F(EnumUtilTests, enumMinValueIsCorrect){	
	EXPECT_EQ(util::enum_min_value<TestEnum>(), 0);
	EXPECT_EQ(util::enum_min_value<ScopedEnum>(), 0);
	EXPECT_EQ(util::enum_min_value<TestEnum2>(), 10);
}

TEST_F(EnumUtilTests, enumValuesCorrectlyValid){
	EXPECT_TRUE(util::enum_value_valid(v0));
	EXPECT_TRUE(util::enum_value_valid(ScopedEnum::s0));
	EXPECT_TRUE(util::enum_value_valid(t4));
}

TEST_F(EnumUtilTests, enumValuesCorrectlyInvalid){
	int val = 15;
	EXPECT_FALSE(util::enum_value_valid(static_cast<TestEnum>(val)));
	EXPECT_FALSE(util::enum_value_valid(static_cast<ScopedEnum>(val)));
	EXPECT_FALSE(util::enum_value_valid(static_cast<TestEnum2>(val)));
}

TEST_F(EnumUtilTests, enumNameCorrect){
	EXPECT_EQ(util::enum_name(v0), "v0");
	EXPECT_EQ(util::enum_name(t4), "t4");
	EXPECT_EQ(util::enum_name(ScopedEnum::s2), "s2");
	EXPECT_EQ(util::enum_name(ScopedEnum2::s3), "s3");
}

TEST_F(EnumUtilTests, enumNamesCorrect){
	using TArr = std::array<std::string_view, 5>;

	TArr unscoped_names{"v0","v1","v2","v3","v4"};
	TArr scoped_names{"s0","s2","s3","s4","s1"};
	TArr scoped_names2{"s0","s1","s2","s3","s4"};
	EXPECT_EQ(util::enum_names<TestEnum>(), unscoped_names);
	EXPECT_EQ(util::enum_names<ScopedEnum>(), scoped_names);
	EXPECT_EQ(util::enum_names<ScopedEnum2>(), scoped_names2);
}

TEST_F(EnumUtilTests, enumValuesLengthCorrect){
	EXPECT_EQ(util::enum_values<TestEnum>().max_size(), 5);
	EXPECT_EQ(util::enum_values<ScopedEnum>().max_size(), 5);
	EXPECT_EQ(util::enum_values<TestEnum2>().max_size(), 5);
	EXPECT_EQ(util::enum_values<ScopedEnum2>().max_size(), 5);
}

TEST_F(EnumUtilTests, enumValuesCorrect){
	auto array = std::array<TestEnum,5>{v0,v1,v2,v3,v4};
	EXPECT_EQ(util::enum_values<TestEnum>(), array);
}

TEST_F(EnumUtilTests, enumValuesAreSorted){
	auto array2 = std::array<ScopedEnum,5>{ScopedEnum::s0,ScopedEnum::s1,ScopedEnum::s2,ScopedEnum::s3,ScopedEnum::s4};
	auto enum_vals = util::enum_values<ScopedEnum>();
	EXPECT_NE(enum_vals, array2);
	std::sort(array2.begin(), array2.end());
	EXPECT_EQ(enum_vals, array2);
}

TEST_F(EnumUtilTests, enumUnderlyingValuesCorrect){
	auto array = std::array<JEvent,5>{0,1,2,3,4};
	EXPECT_EQ(util::enum_underlying_values<TestEnum>(), array);
	auto array2 = std::array<unsigned int,5>{10,11,12,13,255};
	EXPECT_EQ(util::enum_underlying_values<TestEnum2>(), array2);

}

TEST_F(EnumUtilTests, strToEnumReturnsCorrectValue){
	std::string_view val_string = "v2";
	auto enum_value = util::enum_from_str<TestEnum>(val_string);
	ASSERT_TRUE(enum_value);
	EXPECT_EQ(*enum_value, v2);
}

TEST_F(EnumUtilTests, strToEnumClassReturnsCorrectValue){
	std::string_view val_string = "s3";
	auto enum_value = util::enum_from_str<ScopedEnum>(val_string);
	ASSERT_TRUE(enum_value);
	EXPECT_EQ(*enum_value, ScopedEnum::s3);
}

TEST_F(EnumUtilTests, invalidStrToEnumReturnsEmptyValue){
	std::string_view val_string = "foo";
	auto enum_value = util::enum_from_str<TestEnum>(val_string);
	EXPECT_FALSE(enum_value.has_value());
}