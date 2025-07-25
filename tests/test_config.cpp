#include "Config.hpp"
#include <gtest/gtest.h>
#include <fstream>


static const std::string testConfig = R"({
  "udp_ip": "127.0.0.1",
  "udp_port": 1234,
  "session_timeout_sec": 45,
  "cdr_file": "testcdr.log",
  "http_port": 9090,
  "graceful_shutdown_rate": 5,
  "log_file": "test.log",
  "log_level": "debug",
  "blacklist": ["A","B"]
})";

TEST(Config, LoadCorrectly) {
    std::ofstream("test_config.json") << testConfig;
    Config c = Config::load("test_config.json");
    EXPECT_EQ(c.udp_ip, "127.0.0.1");
    EXPECT_EQ(c.udp_port, 1234);
    EXPECT_EQ(c.session_timeout_sec, 45);
    EXPECT_EQ(c.cdr_file, "testcdr.log");
    EXPECT_EQ(c.http_port, 9090);
    EXPECT_EQ(c.graceful_shutdown_rate, 5);
    EXPECT_EQ(c.log_file, "test.log");
    EXPECT_EQ(c.log_level, "debug");
    ASSERT_EQ(c.blacklist.size(), 2);
    EXPECT_EQ(c.blacklist[0], "A");
    EXPECT_EQ(c.blacklist[1], "B");
}

TEST(Config, LoadInvalidJson) {
    std::ofstream("bad.json") << "{ invalid json}";
    EXPECT_THROW(Config::load("bad.json"), std::exception);
}

TEST(Config, EmptyBlacklist) {
    std::ofstream("empty.json") << R"({
  "udp_ip": "1",
  "udp_port": 2,
  "session_timeout_sec": 3,
  "cdr_file": "x",
  "http_port": 4,
  "graceful_shutdown_rate": 6,
  "log_file": "y",
  "log_level": "info",
  "blacklist": []
})";
    Config c = Config::load("empty.json");
    EXPECT_TRUE(c.blacklist.empty());
}

TEST(Config, MissingField) {
    std::ofstream("miss.json") << R"({
  "udp_ip": "127.0.0.1"
})";
    EXPECT_THROW(Config::load("miss.json"), std::exception);
}

TEST(Config, AdditionalFieldsIgnored) {
    std::ofstream("extra.json") << R"({
  "udp_ip": "ip",
  "udp_port": 1,
  "session_timeout_sec": 2,
  "cdr_file": "f",
  "http_port": 3,
  "graceful_shutdown_rate": 4,
  "log_file": "l",
  "log_level": "d",
  "blacklist": ["X"],
  "extra_field": "ignored"
})";
    Config c = Config::load("extra.json");
    EXPECT_EQ(c.udp_ip, "ip");
    EXPECT_EQ(c.blacklist[0], "X");
}