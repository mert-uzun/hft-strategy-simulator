# Minimum CMake version required
cmake_minimum_required(VERSION 3.14)

# Project Name
project(LatencyAwareOrderBookSim)

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

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


# Include FetchContent module
include(FetchContent)

# Fetch pybind11
FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v2.12.1
)
FetchContent_MakeAvailable(pybind11)

pybind11_add_module(orderbook_wrapper bindings/pybind.bindings.cpp)

target_link_libraries(orderbook_wrapper PRIVATE OrderBookLib)

# Tell CMake to download version 1.14.0 of GoogleTest
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

# On Windows, this prevents overriding parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Now download and make GoogleTest available for CMake
FetchContent_MakeAvailable(googletest)

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