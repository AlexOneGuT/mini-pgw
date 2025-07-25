#include "Server.hpp"
#include <iostream>
#include <spdlog/spdlog.h>

int main() {
    try {
        Server srv("configs/server_config.json");
        srv.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;

        try {
            spdlog::critical("Fatal error during startup: {}", e.what());
        }
        catch (...) {
        }
        return 1;
    }
    return 0;
}
