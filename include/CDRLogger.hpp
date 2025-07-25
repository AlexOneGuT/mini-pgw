#pragma once
#include <string>
#include <fstream>
#include <mutex>

class CDRLogger {
public:
  CDRLogger(const std::string& path);
  void log(const std::string& imsi, const std::string& action);
private:
  std::ofstream out;
  std::mutex mtx;
};
