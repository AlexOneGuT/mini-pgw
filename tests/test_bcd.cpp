#include "BCDHelper.hpp"
#include <gtest/gtest.h>

TEST(BCDTest, EncodeDecodeEven) {
	std::string imsi = "123456789012345";
	auto bcd = string_to_bcd(imsi);
	std::string decoded = bcd_to_string(bcd.data(), bcd.size());
	EXPECT_EQ(imsi, decoded);
}

TEST(BCDTest, EncodeDecodeOdd) {
	std::string imsi = "12345678901234";
	auto bcd = string_to_bcd(imsi);
	std::string decoded = bcd_to_string(bcd.data(), bcd.size());
	EXPECT_EQ(imsi, decoded);
}

TEST(BCDTest, InvalidIMSI) {
	EXPECT_THROW(string_to_bcd("123a56"), std::invalid_argument);
}

TEST(BCDTest, EmptyIMSI) {
	auto bcd = string_to_bcd("");
	EXPECT_TRUE(bcd.empty());
	EXPECT_EQ(bcd_to_string(nullptr, 0), "");
}
TEST(BCDTest, EncodeDecodeBoundary) {
    auto empty_bcd = string_to_bcd("");
    EXPECT_TRUE(empty_bcd.empty());
    EXPECT_EQ(bcd_to_string(empty_bcd.data(), empty_bcd.size()), "");

    std::string max_imsi = "123456789012345";
    auto max_bcd = string_to_bcd(max_imsi);
    EXPECT_EQ(max_bcd.size(), 8);
    EXPECT_EQ(bcd_to_string(max_bcd.data(), max_bcd.size()), max_imsi);
}