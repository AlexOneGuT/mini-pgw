#include "Config.hpp"
#include <fstream>
#include <stdexcept>
#include <spdlog/spdlog.h>
#include <iostream>

using json = nlohmann::json;

Config Config::load(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cout << "Failed to open. Current directory: " << std::filesystem::current_path() << std::endl;
        throw std::runtime_error("Cannot open config file: " + path);
    }

    try {
        json j;
        f >> j;

        Config c;
        c.udp_ip = j["udp_ip"];
        c.udp_port = j["udp_port"];
        c.session_timeout_sec = j["session_timeout_sec"];
        c.cdr_file = j["cdr_file"];
        c.http_port = j["http_port"];
        c.graceful_shutdown_rate = j["graceful_shutdown_rate"];
        c.log_file = j["log_file"];
        c.log_level = j["log_level"];
        c.blacklist = j["blacklist"].get<std::vector<std::string>>();

        spdlog::debug("Config loaded successfully: {}", path);
        return c;
    }
    catch (const json::parse_error& e) {
        spdlog::critical("JSON parse error in {}: {}", path, e.what());
        throw;
    }
    catch (const json::exception& e) {
        spdlog::critical("JSON error in {}: {}", path, e.what());
        throw;
    }
}
