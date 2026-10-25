# Host-side unit tests.
#   cmake -S . -B build && cmake --build build && ctest --test-dir build
cmake_minimum_required(VERSION 3.21)
project(MatchmakingProtocol CXX)

add_executable(mm-protocol-tests tests/main.cpp tests/test_mm_protocol.cpp Protocol.cpp)
target_compile_options(mm-protocol-tests PRIVATE -Wall -Wextra -std=c++98)

enable_testing()
add_test(NAME mm-protocol-tests COMMAND mm-protocol-tests)
