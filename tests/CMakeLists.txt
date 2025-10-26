# Tests CMakeLists.txt for Workspace Window Enhancement

cmake_minimum_required(VERSION 3.16)

# Find required packages
find_package(GTest REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Set up test executable name
set(TEST_TARGET cua_tests)

# Collect all test source files
file(GLOB_RECURSE UNIT_TEST_SOURCES "unit/*.cpp")
file(GLOB_RECURSE INTEGRATION_TEST_SOURCES "integration/*.cpp")

# Collect all source files to test (excluding main.cpp)
file(GLOB_RECURSE SRC_FILES
    "${CMAKE_SOURCE_DIR}/src/core/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/filters/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/ui/*.cpp"
)

# Remove main.cpp if it exists
list(FILTER SRC_FILES EXCLUDE REGEX ".*main\\.cpp$")

# Create test executable
add_executable(${TEST_TARGET}
    ${UNIT_TEST_SOURCES}
    ${INTEGRATION_TEST_SOURCES}
    ${SRC_FILES}
    test_main.cpp
)

# Link libraries
target_link_libraries(${TEST_TARGET}
    GTest::GTest
    GTest::Main
    ${PLATFORM_LIBS}
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(${TEST_TARGET}
        user32 kernel32 psapi shell32 ole32 oleaut32
    )
elseif(APPLE)
    find_library(COREGRAPHICS_LIBRARY CoreGraphics)
    find_library(FOUNDATION_LIBRARY Foundation)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(CARBON_LIBRARY Carbon)
    find_library(ACCESSIBILITY_LIBRARY ApplicationServices)

    target_link_libraries(${TEST_TARGET}
        ${COREGRAPHICS_LIBRARY}
        ${FOUNDATION_LIBRARY}
        ${COCOA_LIBRARY}
        ${CARBON_LIBRARY}
        ${ACCESSIBILITY_LIBRARY}
    )
elseif(UNIX)
    find_package(X11 REQUIRED)
    target_link_libraries(${TEST_TARGET}
        ${X11_LIBRARIES}
        ${X11_Xext_LIB}
    )
endif()

# Set C++17 standard
set_target_properties(${TEST_TARGET} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Enable testing
enable_testing()

# Add test discovery
include(GoogleTest)
gtest_discover_tests(${TEST_TARGET})

# Custom test targets
add_custom_target(test_unit
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose -R "unit"
    DEPENDS ${TEST_TARGET}
    COMMENT "Running unit tests"
)

add_custom_target(test_integration
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose -R "integration"
    DEPENDS ${TEST_TARGET}
    COMMENT "Running integration tests"
)

add_custom_target(test_compatibility
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose -R "Compatibility"
    DEPENDS ${TEST_TARGET}
    COMMENT "Running compatibility tests"
)

add_custom_target(test_workspace
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose -R "Workspace"
    DEPENDS ${TEST_TARGET}
    COMMENT "Running workspace management tests"
)

add_custom_target(test_performance
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose -R "Performance"
    DEPENDS ${TEST_TARGET}
    COMMENT "Running performance tests"
)

# Test with coverage (if gcov available)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

    if(ENABLE_COVERAGE)
        target_compile_options(${TEST_TARGET} PRIVATE --coverage)
        target_link_options(${TEST_TARGET} PRIVATE --coverage)

        add_custom_target(coverage
            COMMAND lcov --directory . --capture --output-file coverage.info
            COMMAND lcov --remove coverage.info '/usr/*' --output-file coverage.info
            COMMAND lcov --list coverage.info
            COMMAND genhtml -o coverage coverage.info
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating coverage report"
        )
    endif()
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(${TEST_TARGET} PRIVATE /W4)
else()
    target_compile_options(${TEST_TARGET} PRIVATE -Wall -Wextra -pedantic)
endif()

# Add custom test configurations
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/test_config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/test_config.h
)

target_include_directories(${TEST_TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Install test executable (optional)
install(TARGETS ${TEST_TARGET}
    RUNTIME DESTINATION bin/tests
)

# CTest configuration
set(CTEST_OUTPUT_ON_FAILURE TRUE)
set(CTEST_PARALLEL_LEVEL 4)

message(STATUS "Tests configured:")
message(STATUS "  Unit tests: ${UNIT_TEST_SOURCES}")
message(STATUS "  Integration tests: ${INTEGRATION_TEST_SOURCES}")
message(STATUS "  Test target: ${TEST_TARGET}")
message(STATUS "  Coverage enabled: ${ENABLE_COVERAGE}")