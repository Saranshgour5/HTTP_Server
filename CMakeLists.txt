cmake_minimum_required(VERSION 3.10)
project(Http_Server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SERVER_LOGIC_SOURCES 
    "src/request_parser.cpp"
)

add_library(server_logic STATIC ${SERVER_LOGIC_SOURCES})
target_include_directories(server_logic PUBLIC include)

find_package(GTest REQUIRED)
find_package(Boost REQUIRED)

add_executable(run_tests tests/request_parser_test.cpp)

target_link_libraries(run_tests 
    PRIVATE 
        server_logic 
        GTest::gtest 
        GTest::Main
)

enable_testing()
add_test(NAME AllTests COMMAND run_tests)
