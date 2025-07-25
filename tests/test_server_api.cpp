#include "Server.hpp"
#include <gtest/gtest.h>
#include <httplib.h>
#include <thread>
#include <filesystem>
#include <fstream>

class ServerTest : public ::testing::Test {
protected:
    Server* server;
    std::thread server_thread;
    const int HTTP_PORT = 8070;
    const std::string CONFIG_PATH = "test_server_config.json";
    const std::string CDR_PATH = "logs/test_cdr.log";
    const std::string LOG_PATH = "logs/test_pgw.log";

    void SetUp() override {
        std::ofstream cfg_file(CONFIG_PATH);
        cfg_file << R"({
            "udp_ip": "127.0.0.1",
            "udp_port": 9001,
            "session_timeout_sec": 1,
            "cdr_file": ")" << CDR_PATH << R"(",
            "http_port": 8070,
            "graceful_shutdown_rate": 1,
            "log_file": ")" << LOG_PATH << R"(",
            "log_level": "off",
            "blacklist": []
        })";
        cfg_file.close();

        std::filesystem::remove(CDR_PATH);
        std::filesystem::remove(LOG_PATH);

        server = new Server(CONFIG_PATH);
        server_thread = std::thread([this] { server->run(); });
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    void TearDown() override {
        if (!shuttingDown) {
            httplib::Client cli("localhost", HTTP_PORT);
            auto res = cli.Get("/stop");
        }
        
        if (server_thread.joinable()) {
            server_thread.join();
        }
        
        delete server;
        
        // Очистка тестовых файлов
        std::filesystem::remove(CONFIG_PATH);
        std::filesystem::remove(CDR_PATH);
        std::filesystem::remove(LOG_PATH);
    }

    bool shuttingDown = false;
};

TEST_F(ServerTest, CheckSubscriberNotActive) {
    httplib::Client cli("localhost", HTTP_PORT);
    auto res = cli.Get("/check_subscriber?imsi=123456789123456");
    
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);
    EXPECT_EQ(res->body, "not active");
}

TEST_F(ServerTest, ReloadConfig) {
    httplib::Client cli("localhost", HTTP_PORT);
    auto res = cli.Get("/reload_config");
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);
    EXPECT_EQ(res->body, "config reloaded");
}

TEST_F(ServerTest, StopEndpoint) {
    shuttingDown = true;
    httplib::Client cli("localhost", HTTP_PORT);
    auto res = cli.Get("/stop");
    
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);
    EXPECT_EQ(res->body, "shutting down");
}