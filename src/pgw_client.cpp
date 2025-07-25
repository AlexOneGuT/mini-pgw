#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include "BCDHelper.hpp"
#include <spdlog/sinks/basic_file_sink.h>
#include <cstdint>

using json = nlohmann::json;

struct ClientConfig {
    std::string server_ip;
    int server_port;
    std::string log_file;
    std::string log_level;

    static ClientConfig load(const std::string& p) {
        std::ifstream f(p);
        if (!f.is_open()) {
            throw std::runtime_error("Cannot open config file: " + p);
        }
        json j;
        f >> j;
        return {
            j["server_ip"].get<std::string>(),
            j["server_port"].get<int>(),
            j["log_file"].get<std::string>(),
            j["log_level"].get<std::string>()
        };
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: pgw_client <IMSI>\n";
        return 1;
    }

    std::string imsi = argv[1];

    if (imsi.empty() || (imsi.size() > 15 || imsi.size() < 10)) {
        std::cerr << "Invalid IMSI format\n";
        return 1;
    }

    for (char c : imsi) {
        if (c < '0' || c > '9') {
            std::cerr << "IMSI must contain only digits\n";
            return 1;
        }
    }

    try {
        ClientConfig cfg = ClientConfig::load("configs/client_config.json");

        try {
            auto file_logger = spdlog::basic_logger_mt("client_logger", cfg.log_file);
            spdlog::set_default_logger(file_logger);
        } catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Log init failed: " << ex.what() << std::endl;
            return 1;
        }

        spdlog::set_level(spdlog::level::from_str(cfg.log_level));
        spdlog::info("Sending IMSI {}", imsi);

        std::vector<uint8_t> bcd_data = string_to_bcd(imsi);

        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            spdlog::critical("Failed to create socket: {}", strerror(errno));
            return 1;
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(cfg.server_port);

        if (inet_pton(AF_INET, cfg.server_ip.c_str(), &addr.sin_addr) <= 0) {
            spdlog::error("Invalid server IP address");
            close(sock);
            return 1;
        }

        ssize_t sent = sendto(sock, bcd_data.data(), bcd_data.size(), 0,
                             (sockaddr*)&addr, sizeof(addr));

        if (sent < 0) {
            spdlog::error("Failed to send data: {}", strerror(errno));
            close(sock);
            return 1;
        }

        char buf[32];
        socklen_t addrLen = sizeof(addr);
        ssize_t len = recv(sock, buf, sizeof(buf)-1, 0);

        if (len > 0) {
            buf[len] = '\0';
            std::cout << buf << std::endl;
            spdlog::info("Response: {}", buf);
        } else {
            spdlog::error("No response: {}", strerror(errno));
        }

        close(sock);
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}