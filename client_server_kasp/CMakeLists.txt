cmake_minimum_required(VERSION 3.10)
project(kasp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(THIRDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
set(GTEST_INCLUDE_DIR ${THIRDPARTY_DIR}/googletest/googletest/include)
set(GTEST_LIB_DIR ${THIRDPARTY_DIR}/googletest/build/lib)

add_executable(server
    src/server/main.cpp
    src/server/Server.cpp
    src/server/ClientHandler.cpp
    src/server/Config.cpp
    src/network/Socket.cpp
)

add_executable(client
    src/client/main.cpp
    src/client/Client.cpp
    src/network/Socket.cpp
)

add_executable(stats
    src/stats/StatsUtil.cpp
)

add_executable(tests
    tests/unit_tests.cpp
    src/server/ClientHandler.cpp
    src/server/Config.cpp
    src/network/Socket.cpp
)

target_include_directories(server PRIVATE include ${THIRDPARTY_DIR})
target_include_directories(client PRIVATE  include ${THIRDPARTY_DIR})
target_include_directories(stats PRIVATE  include  ${THIRDPARTY_DIR})
target_include_directories(tests PRIVATE include ${THIRDPARTY_DIR} ${GTEST_INCLUDE_DIR})

target_link_libraries(server pthread)
target_link_libraries(client pthread)
target_link_libraries(stats pthread)

target_link_libraries(tests
pthread
${GTEST_LIB_DIR}/libgtest.a
${GTEST_LIB_DIR}/libgtest_main.a
)


