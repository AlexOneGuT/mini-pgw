#include "SessionManager.hpp"
#include <gtest/gtest.h>
#include <thread>
#include <spdlog/spdlog.h>

static const std::string IMSI1 = "001010123456789";
static const std::string IMSI2 = "001010123456788";
static const std::string IMSI3 = "001010123456787";

class SessionManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        spdlog::set_level(spdlog::level::off);
    }
};

TEST_F(SessionManagerTest, BasicOperations) {
    SessionManager m(5);
    
 
    EXPECT_FALSE(m.exists(IMSI1));
    EXPECT_TRUE(m.create(IMSI1));
    EXPECT_TRUE(m.exists(IMSI1)); 
    
    EXPECT_FALSE(m.create(IMSI1));
    
    EXPECT_TRUE(m.create(IMSI2));
    EXPECT_TRUE(m.exists(IMSI2));
}

TEST_F(SessionManagerTest, SessionExpiration) {
    SessionManager m(1); 
    
    m.create(IMSI1);
    EXPECT_TRUE(m.exists(IMSI1));
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    m.removeExpired();
    
    EXPECT_FALSE(m.exists(IMSI1));
}

TEST_F(SessionManagerTest, ThreadSafety) {
    SessionManager m(60);
    const int N = 100;
    
    std::thread creator([&] {
        for (int i = 0; i < N; ++i) {
            m.create(IMSI1 + std::to_string(i));
        }
    });
    
    std::thread checker([&] {
        for (int i = 0; i < N; ++i) {
            m.exists(IMSI1 + std::to_string(i));
        }
    });
    
    creator.join();
    checker.join();
    
    EXPECT_TRUE(m.exists(IMSI1 + std::to_string(N/2)));
}

TEST_F(SessionManagerTest, ForceRemove) {
    bool callbackCalled = false;
    SessionManager m(60);
    
    m.setExpiredCallback([&](const std::string& imsi) {
        callbackCalled = (imsi == IMSI1);
    });
    
    m.create(IMSI1);
    m.forceRemove(IMSI1);
    
    EXPECT_TRUE(callbackCalled);
    EXPECT_FALSE(m.exists(IMSI1));
    EXPECT_EQ(m.sessionCount(), 0);
}

TEST_F(SessionManagerTest, GetAllSessions) {
    SessionManager m(60);
    
    m.create(IMSI1);
    m.create(IMSI2);
    
    auto sessions = m.getAllSessions();
    EXPECT_EQ(sessions.size(), 2);
    EXPECT_NE(std::find(sessions.begin(), sessions.end(), IMSI1), sessions.end());
    EXPECT_NE(std::find(sessions.begin(), sessions.end(), IMSI2), sessions.end());
}