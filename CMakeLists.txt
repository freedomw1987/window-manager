cmake_minimum_required(VERSION 3.16)
project(window-manager VERSION 1.0.0 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

# Platform detection
if(WIN32)
    add_definitions(-DWINDOWS_PLATFORM)
    set(PLATFORM_SOURCES
        src/platform/windows/win32_enumerator.cpp
    )
    set(PLATFORM_LIBS user32 kernel32 psapi shell32 ole32 oleaut32)
elseif(APPLE)
    add_definitions(-DMACOS_PLATFORM)
    set(PLATFORM_SOURCES
        src/platform/macos/cocoa_enumerator.cpp
    )
    find_library(COREGRAPHICS_LIBRARY CoreGraphics)
    find_library(FOUNDATION_LIBRARY Foundation)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(CARBON_LIBRARY Carbon)
    find_library(ACCESSIBILITY_LIBRARY ApplicationServices)
    set(PLATFORM_LIBS ${COREGRAPHICS_LIBRARY} ${FOUNDATION_LIBRARY} ${COCOA_LIBRARY} ${CARBON_LIBRARY} ${ACCESSIBILITY_LIBRARY})
elseif(UNIX)
    add_definitions(-DLINUX_PLATFORM)
    set(PLATFORM_SOURCES
        src/platform/linux/x11_enumerator.cpp
    )
    find_package(X11 REQUIRED)
    set(PLATFORM_LIBS ${X11_LIBRARIES} ${X11_Xext_LIB})
    include_directories(${X11_INCLUDE_DIR})
endif()

# Common source files (User Stories 1, 2, and 3)
set(CORE_SOURCES
    src/core/window.cpp
    src/core/enumerator.cpp
    src/core/window_manager.cpp
    src/core/workspace.cpp
    src/core/exceptions.cpp
    src/ui/cli.cpp
    src/ui/interactive.cpp
    src/filters/search_query.cpp
    src/filters/filter_result.cpp
    src/filters/filter.cpp
    src/main.cpp
)

# Main executable
add_executable(window-manager
    ${CORE_SOURCES}
    ${PLATFORM_SOURCES}
)

# Link platform-specific libraries
target_link_libraries(window-manager ${PLATFORM_LIBS})

# Compiler-specific options
if(MSVC)
    target_compile_options(window-manager PRIVATE /W4)
else()
    target_compile_options(window-manager PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Enable testing
enable_testing()

# Google Test integration
option(BUILD_TESTING "Build the testing tree." OFF)

if(BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.15.2
    )

    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # Test executable (User Stories 1, 2, and 3)
    add_executable(window-manager-tests
        # Core sources (excluding main.cpp)
        src/core/window.cpp
        src/core/enumerator.cpp
        src/core/window_manager.cpp
        src/core/workspace.cpp
        src/core/exceptions.cpp
        src/ui/cli.cpp
        src/ui/interactive.cpp
        src/filters/search_query.cpp
        src/filters/filter_result.cpp
        src/filters/filter.cpp
        ${PLATFORM_SOURCES}
    )

    target_link_libraries(window-manager-tests
        gtest_main
        gmock_main
        ${PLATFORM_LIBS}
    )

    include(GoogleTest)
    gtest_discover_tests(window-manager-tests)
endif()

# FTXUI integration for interactive terminal UI
option(BUILD_FTXUI "Build with FTXUI for interactive UI." ON)

if(BUILD_FTXUI)
    include(FetchContent)
    FetchContent_Declare(ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
        GIT_TAG v5.0.0
    )

    FetchContent_MakeAvailable(ftxui)

    # Link FTXUI to main executable
    target_link_libraries(window-manager
        ftxui::screen
        ftxui::dom
        ftxui::component
    )

    # Also link to tests if building tests
    if(BUILD_TESTING)
        target_link_libraries(window-manager-tests
            ftxui::screen
            ftxui::dom
            ftxui::component
        )
    endif()
endif()

# Installation
install(TARGETS window-manager
    RUNTIME DESTINATION bin
)

# Generate compile_commands.json for IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)