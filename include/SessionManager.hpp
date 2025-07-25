#pragma once
#include <string>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <functional>
#include <vector>

class SessionManager {
public:
	using Callback = std::function<void(const std::string&)>;

	SessionManager(int timeout_sec);
	void setTimeout(int sec);
	void setExpiredCallback(Callback cb);
	bool create(const std::string& imsi);
	bool exists(const std::string& imsi) const;
	void removeExpired();
	void forceRemove(const std::string& imsi);
	size_t sessionCount() const;
	std::vector<std::string> getAllSessions() const;


private:
	int timeout;
	std::unordered_map<std::string, std::chrono::steady_clock::time_point> sessions;
	mutable std::mutex mtx;
	Callback expiredCallback;
};
