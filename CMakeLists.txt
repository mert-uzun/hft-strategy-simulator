# Minimum CMake version required
cmake_minimum_required(VERSION 3.14)

# Project Name
project(LatencyAwareOrderBookSim)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 REQUIRED)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find all header and source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

# Log the found source files
message(STATUS "Found ${CMAKE_CURRENT_SOURCE_DIR}/src files")
foreach(SOURCE ${SOURCES})
    message(STATUS "    ${SOURCE}")
endforeach(SOURCE ${SOURCES})


# Add source and header files to a library
add_library(OrderBookLib ${SOURCES} ${HEADERS})

# Tell the compiler where to look for header files
target_include_directories(OrderBookLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

pybind11_add_module(orderbook_wrapper bindings/pybind.bindings.cpp)

target_link_libraries(orderbook_wrapper PRIVATE OrderBookLib)
target_link_libraries(orderbook_wrapper PRIVATE pybind11::tools)

find_package(GTest REQUIRED)

# Enable testing
enable_testing()

# Create an executable with name AllTests from the code test_orderbook_cpp
add_executable(AllTests
    tests/test_orderbook.cpp
    tests/test_metrics.cpp
    tests/test_latency.cpp
    tests/test_strategy.cpp
    tests/test_market_engine.cpp
)

# Link the test executable with the library and GoogleTests framework + main
target_link_libraries(AllTests
    OrderBookLib
    GTest::gtest_main
)

# Include GoogleTest
# gtest_discover_tests automatically discovers all TEST() macros in given test files
include (GoogleTest)
gtest_discover_tests(AllTests)

# OPTIONAL - Create an executable named main.cpp
add_executable(LatencyAwareOrderBookSim main.cpp)
target_link_libraries(LatencyAwareOrderBookSim OrderBookLib)