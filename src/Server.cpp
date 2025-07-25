#include "Server.hpp"
#include "BCDHelper.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <algorithm>
#include <filesystem>

Server::Server(const std::string& configPath)
    : configPath(std::filesystem::absolute(configPath).string()), 
      cfg(Config::load(configPath)),
      sessMgr(cfg.session_timeout_sec), 
      cdr(cfg.cdr_file),
      blacklist(cfg.blacklist.begin(), cfg.blacklist.end())
{
    std::filesystem::path log_path(cfg.log_file);
    std::filesystem::create_directories(log_path.parent_path());

    try {
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(cfg.log_file, true);
        auto logger = std::make_shared<spdlog::logger>("server_logger", sink);
        logger->set_level(spdlog::level::from_str(cfg.log_level));
        logger->flush_on(spdlog::level::info);
        spdlog::set_default_logger(logger);
        
        spdlog::info("PGW server starting. Log file: {}", cfg.log_file);
    }
    catch (const std::exception& ex) {
        std::cerr << "Log init failed: " << ex.what() << "\n";
        throw;
    }

    sessMgr.setExpiredCallback([this](const std::string& imsi) {
        cdr.log(imsi, "timeout");
    });

    setupHttp();
}

void Server::loadConfig() {
    spdlog::info("Reloading config...");
    cfg = Config::load(configPath);
    sessMgr.setTimeout(cfg.session_timeout_sec);
    spdlog::set_level(spdlog::level::from_str(cfg.log_level));
    
    blacklist.clear();
    blacklist.insert(cfg.blacklist.begin(), cfg.blacklist.end());
    
    spdlog::info("Config reloaded");
}

void Server::setupHttp() {
    http.Get("/check_subscriber", [&](auto &req, auto &res) {
        auto imsi = req.get_param_value("imsi");
        res.set_content(sessMgr.exists(imsi) ? "active" : "not active", "text/plain");
    });
    
    http.Get("/stop", [&](auto&, auto& res){
        shuttingDown = true;
        res.set_content("shutting down", "text/plain");
    });
    
    http.Get("/reload_config", [&](auto&, auto& res) {
        loadConfig();
        res.set_content("config reloaded", "text/plain");
    });
}

void Server::run() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        spdlog::critical("Failed to create UDP socket: {}", strerror(errno));
        return;
    }
    
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(cfg.udp_port);
    addr.sin_addr.s_addr = inet_addr(cfg.udp_ip.c_str());
    
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        spdlog::critical("Failed to bind UDP socket: {}", strerror(errno));
        close(sock);
        return;
    }
    
    httpThread = std::thread([this] {
        try {
            spdlog::info("HTTP server starting on port {}", cfg.http_port);
            http.listen("0.0.0.0", cfg.http_port);
        } catch (const std::exception& e) {
            spdlog::error("HTTP server error: {}", e.what());
        }
    });
    
    char buf[32];
    sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    
    struct timeval tv{};
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    spdlog::info("Starting UDP server on {}:{}", cfg.udp_ip, cfg.udp_port);
    
    while (!shuttingDown) {
        ssize_t len = recvfrom(sock, buf, sizeof(buf), 0,
                (sockaddr*)&clientAddr, &addrLen);
        
        if (len > 0) {
            try {
                std::string imsi = bcd_to_string(
                    reinterpret_cast<uint8_t*>(buf), 
                    static_cast<size_t>(len)
                );
                
                if (imsi.empty() || imsi.size() > 15 || imsi.size() < 10) {
                    spdlog::warn("Invalid IMSI received: {}", imsi);
                    sendto(sock, "rejected", 8, 0, (sockaddr*)&clientAddr, addrLen);
                    continue;
                }
                
                if (blacklist.find(imsi) != blacklist.end()) {
                    sendto(sock, "rejected", 8, 0, (sockaddr*)&clientAddr, addrLen);
                    cdr.log(imsi, "rejected_blacklist");
                    spdlog::info("IMSI {} rejected (blacklisted)", imsi);
                } 
                else if (sessMgr.create(imsi)) {
                    sendto(sock, "created", 7, 0, (sockaddr*)&clientAddr, addrLen);
                    cdr.log(imsi, "created");
                    spdlog::info("Session created for IMSI {}", imsi);
                } 
                else {
                    sendto(sock, "rejected", 8, 0, (sockaddr*)&clientAddr, addrLen);
                    cdr.log(imsi, "rejected_duplicate");
                    spdlog::warn("Session already exists for IMSI {}", imsi);
                }
            } catch (const std::exception& e) {
                spdlog::error("Error processing packet: {}", e.what());
            }
        }
        
        static auto lastCheck = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        if (now - lastCheck > std::chrono::seconds(5)) {
            sessMgr.removeExpired();
            lastCheck = now;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    spdlog::info("Starting graceful shutdown");
    sessMgr.setExpiredCallback(nullptr);
    
    while (sessMgr.sessionCount() > 0) {
        auto sessions = sessMgr.getAllSessions();
        size_t count = 0;
        
        for (const auto& imsi : sessions) {
            if (count >= cfg.graceful_shutdown_rate) break;
            
            cdr.log(imsi, "shutdown");
            sessMgr.forceRemove(imsi);
            count++;
        }
        
        std::this_thread::sleep_for(
            std::chrono::milliseconds(1000 / cfg.graceful_shutdown_rate)
        );
    }
    
    close(sock);
    http.stop();
    
    if (httpThread.joinable()) {
        httpThread.join();
    }
    
    spdlog::info("PGW server stopped");
}