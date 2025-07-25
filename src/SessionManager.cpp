#include "SessionManager.hpp"
#include <algorithm>
#include <spdlog/spdlog.h>

SessionManager::SessionManager(int timeout_sec)
    : timeout(timeout_sec) {}

void SessionManager::setTimeout(int sec) {
    std::lock_guard lk(mtx);
    timeout = sec;
}

void SessionManager::setExpiredCallback(Callback cb) {
    std::lock_guard lk(mtx);
    expiredCallback = std::move(cb);
}

bool SessionManager::create(const std::string& imsi) {
    std::lock_guard lk(mtx);
    if (sessions.count(imsi)) return false;
    sessions[imsi] = std::chrono::steady_clock::now();
    return true;
}

bool SessionManager::exists(const std::string& imsi) const {
    std::lock_guard lk(mtx);
    return sessions.count(imsi);
}

void SessionManager::removeExpired() {
    if (sessions.empty()) return;

    if (timeout <= 0) {
        spdlog::warn("Invalid timeout value: {}", timeout);
        return;
    }
    const auto now = std::chrono::steady_clock::now();
    const auto threshold = now - std::chrono::seconds(timeout);
    size_t removed_count = 0;
    {
        std::lock_guard lk(mtx);

        for (auto it = sessions.begin(); it != sessions.end();) {
            if (it->second < threshold) {
                try {
                    if (expiredCallback) {
                        expiredCallback(it->first);
                    }
                }
                catch (const std::exception& e) {
                    spdlog::error("Callback error for IMSI {}: {}", it->first, e.what());
                }
                it = sessions.erase(it);
                removed_count++;
            }
            else {
                ++it;
            }
        }
    }
    
    if (removed_count > 0) {
        spdlog::debug("Removed {} expired sessions", removed_count);
    }
}

void SessionManager::forceRemove(const std::string& imsi) {
    std::lock_guard lk(mtx);
    auto it = sessions.find(imsi);
    if (it != sessions.end()) {
        if (expiredCallback) {
            expiredCallback(imsi);
        }
        sessions.erase(it);
        spdlog::info("Force removed session for IMSI {}", imsi);
    }
}

size_t SessionManager::sessionCount() const {
    std::lock_guard lk(mtx);
    return sessions.size();
}

std::vector<std::string> SessionManager::getAllSessions() const {
    std::lock_guard<std::mutex> lk(mtx);
    std::vector<std::string> result;
    result.reserve(sessions.size());
    
    for (const auto& pair : sessions) {
        result.push_back(pair.first);
    }
    
    return result;
}