cmake_minimum_required(VERSION 3.14)
project(kasp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")


#TODO: походу запихнуть в thirdparty nlohjson надj
include(FetchContent)
FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz  
  DOWNLOAD_EXTRACT_TIMESTAMP YES  
)
FetchContent_MakeAvailable(json)

add_library(network STATIC
    src/network/Socket.cpp
)
target_include_directories(network PUBLIC include)
target_link_libraries(network PUBLIC nlohmann_json::nlohmann_json)

add_library(server_lib STATIC
    src/server/Server.cpp
    src/server/ClientHandler.cpp
    src/server/Config.cpp
)
target_include_directories(server_lib PUBLIC include)
target_link_libraries(server_lib network nlohmann_json::nlohmann_json pthread)

add_library(client_lib STATIC
    src/client/Client.cpp
)
target_include_directories(client_lib PUBLIC include)
target_link_libraries(client_lib network nlohmann_json::nlohmann_json)

add_executable(server
    src/server/main.cpp
)
target_link_libraries(server server_lib)

add_executable(client
    src/client/main.cpp
)
target_link_libraries(client client_lib)

add_executable(stats
    src/stats/StatsUtil.cpp
)

enable_testing()
find_package(GTest QUIET)



if(GTest_FOUND)
    add_executable(unit_tests
        tests/unit_tests.cpp
    )
    target_include_directories(unit_tests PRIVATE include)
    target_link_libraries(unit_tests GTest::gtest GTest::gtest_main network)
    add_test(NAME UnitTests COMMAND unit_tests)
else()
    include(FetchContent)

    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/release-1.12.1.tar.gz
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(unit_tests
        tests/unit_tests.cpp
    )
    target_include_directories(unit_tests PRIVATE include)
    target_link_libraries(unit_tests gtest gtest_main network)
    add_test(NAME UnitTests COMMAND unit_tests)
endif()
