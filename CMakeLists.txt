cmake_minimum_required(VERSION 3.15)
project(mini-pgw CXX)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG v0.10.7
)
FetchContent_MakeAvailable(httplib)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.11.0
)
FetchContent_MakeAvailable(spdlog)

add_library(pgw_lib
  src/Config.cpp
  src/SessionManager.cpp
  src/CDRLogger.cpp
  src/Server.cpp
  src/BCDHelper.cpp
)

target_include_directories(pgw_lib PUBLIC include)
target_link_libraries(pgw_lib PUBLIC nlohmann_json::nlohmann_json httplib spdlog::spdlog)

add_executable(pgw_server src/pgw_server.cpp)
target_link_libraries(pgw_server PRIVATE pgw_lib)

add_executable(pgw_client src/pgw_client.cpp)
target_link_libraries(pgw_client PRIVATE pgw_lib spdlog::spdlog)

enable_testing()
add_subdirectory(tests)

