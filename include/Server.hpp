#pragma once
#include "Config.hpp"
#include "SessionManager.hpp"
#include "CDRLogger.hpp"
#include <httplib.h>
#include <string>
#include <unordered_set>

class Server {
public:
  Server(const std::string& configPath);
  void run();
private:
  std::string configPath;
  Config cfg;
  SessionManager sessMgr;
  CDRLogger cdr;
  std::unordered_set<std::string> blacklist;
  httplib::Server http;
  std::thread httpThread;
  bool shuttingDown = false;

  void loadConfig();
  void setupHttp();
};
