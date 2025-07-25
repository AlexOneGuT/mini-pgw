#include "CDRLogger.hpp"
#include <chrono>
#include <ctime>

CDRLogger::CDRLogger(const std::string& path) : out(path, std::ios::app) {}

void CDRLogger::log(const std::string& imsi, const std::string& action) {
  std::lock_guard lk(mtx);
  auto now = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(now);
  out << std::ctime(&t) << " " << imsi << ":" << action << std::endl;
}
