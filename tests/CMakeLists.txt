include_directories(
    ${gtest_SOURCE_DIR}/include 
    ${gmock_SOURCE_DIR}/include
    ../include
    ${CMAKE_SOURCE_DIR}/include
)

add_executable(test_session test_session.cpp)
target_link_libraries(test_session 
    PRIVATE 
        pgw_lib 
        GTest::gtest 
        GTest::gtest_main
        spdlog::spdlog 
)
add_test(NAME SessionTest COMMAND test_session)

add_executable(test_config test_config.cpp)
target_link_libraries(test_config 
    PRIVATE 
        pgw_lib 
        GTest::gtest 
        GTest::gtest_main
)
add_test(NAME ConfigTest COMMAND test_config)

add_executable(test_server_api test_server_api.cpp)
target_link_libraries(test_server_api 
    PRIVATE 
        pgw_lib 
        GTest::gtest 
        GTest::gtest_main
        httplib
)
add_test(NAME ServerAPITest COMMAND test_server_api)

add_executable(test_server test_server.cpp)
target_link_libraries(test_server
    PRIVATE
    pgw_lib
    GTest::gtest
    GTest::gtest_main
    httplib::httplib
    nlohmann_json::nlohmann_json
)
target_include_directories(test_server PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME ServerTest COMMAND test_server)

add_executable(load_test load_test.cpp)
target_link_libraries(load_test 
    PRIVATE 
        pgw_lib
)