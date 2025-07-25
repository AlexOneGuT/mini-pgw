#include "Server.hpp"
#include <gtest/gtest.h>
#include <httplib.h>
#include <thread>
#include <filesystem>

class ServerTest : public ::testing::Test {
protected:
    Server* server;
    std::thread server_thread;
    const int HTTP_PORT = 8081;
    const std::string CONFIG_PATH = "test_server_config.json";

    void SetUp() override {
        std::ofstream cfg_file(CONFIG_PATH);
        cfg_file << R"({
            "udp_ip": "127.0.0.1",
            "udp_port": 9002,
            "session_timeout_sec": 1,
            "cdr_file": "test_cdr.log",
            "http_port": )" << HTTP_PORT << R"(,
            "graceful_shutdown_rate": 1,
            "log_file": "/tmp/test_pgw.log",
            "log_level": "off",
            "blacklist": ["123456789"]
        })";
        cfg_file.close();

        server = new Server(CONFIG_PATH);
        server_thread = std::thread([this] { server->run(); });
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    void TearDown() override {
        httplib::Client cli("localhost", HTTP_PORT);
        cli.Get("/stop");
        
        if (server_thread.joinable()) {
            server_thread.join();
        }
        
        delete server;
        std::filesystem::remove(CONFIG_PATH);
    }
};

TEST_F(ServerTest, HttpApiCheckSubscriber) {
    httplib::Client cli("localhost", HTTP_PORT);
    auto res = cli.Get("/check_subscriber?imsi=123456789123456");
    
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);
    EXPECT_EQ(res->body, "not active");
}

TEST_F(ServerTest, UdpBlacklistHandling) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9002);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    sendto(sock, "123456789", 9, 0, (sockaddr*)&addr, sizeof(addr));

    char buf[32];
    socklen_t addrLen = sizeof(addr);
    recvfrom(sock, buf, sizeof(buf), 0, (sockaddr*)&addr, &addrLen);
    
    EXPECT_STREQ(buf, "rejected");
    close(sock);
}

TEST_F(ServerTest, ConfigReload) {
    httplib::Client cli("localhost", HTTP_PORT);
    auto res = cli.Get("/reload_config");
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);
}