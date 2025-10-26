#include <gtest/gtest.h>
#include <iostream>
#include <chrono>

// Test environment setup
class WorkspaceTestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        std::cout << "Setting up Workspace Window Enhancement test environment..." << std::endl;

        // Initialize test timing
        startTime = std::chrono::steady_clock::now();

        // Print system information
        printSystemInfo();

        // Verify basic requirements
        verifyTestRequirements();
    }

    void TearDown() override {
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        std::cout << "Tests completed in " << duration.count() << "ms" << std::endl;
        std::cout << "Tearing down test environment..." << std::endl;
    }

private:
    std::chrono::steady_clock::time_point startTime;

    void printSystemInfo() {
        std::cout << "System Information:" << std::endl;

#ifdef _WIN32
        std::cout << "  Platform: Windows" << std::endl;
        std::cout << "  Workspace API: Virtual Desktop Manager" << std::endl;
#elif __APPLE__
        std::cout << "  Platform: macOS" << std::endl;
        std::cout << "  Workspace API: Core Graphics + Accessibility" << std::endl;
#elif __linux__
        std::cout << "  Platform: Linux" << std::endl;
        std::cout << "  Workspace API: EWMH (X11)" << std::endl;
#else
        std::cout << "  Platform: Unknown" << std::endl;
        std::cout << "  Workspace API: None (fallback mode)" << std::endl;
#endif

        std::cout << "  C++ Standard: " << __cplusplus << std::endl;
        std::cout << "  Compiler: ";

#ifdef __GNUC__
        std::cout << "GCC " << __GNUC__ << "." << __GNUC_MINOR__;
#elif _MSC_VER
        std::cout << "MSVC " << _MSC_VER;
#elif __clang__
        std::cout << "Clang " << __clang_major__ << "." << __clang_minor__;
#else
        std::cout << "Unknown";
#endif
        std::cout << std::endl;
    }

    void verifyTestRequirements() {
        std::cout << "Verifying test requirements:" << std::endl;

        // Check if running in a GUI environment
        bool hasDisplay = false;

#ifdef _WIN32
        hasDisplay = GetDesktopWindow() != nullptr;
#elif __APPLE__
        // macOS always has a display in normal circumstances
        hasDisplay = true;
#elif __linux__
        const char* display = getenv("DISPLAY");
        hasDisplay = (display != nullptr && strlen(display) > 0);
#endif

        if (hasDisplay) {
            std::cout << "  ✓ GUI environment detected" << std::endl;
        } else {
            std::cout << "  ⚠ No GUI environment detected - some tests may be limited" << std::endl;
        }

        // Check for permissions (especially important on macOS)
#ifdef __APPLE__
        std::cout << "  ⚠ Note: macOS Accessibility permissions may be required for full testing" << std::endl;
#endif

        std::cout << "  ✓ Test requirements verified" << std::endl;
    }
};

// Custom test event listener for detailed reporting
class WorkspaceTestListener : public ::testing::TestEventListener {
public:
    void OnTestStart(const ::testing::TestInfo& test_info) override {
        current_test_start_ = std::chrono::steady_clock::now();
        std::cout << "[ STARTING ] " << test_info.test_suite_name()
                  << "." << test_info.name() << std::endl;
    }

    void OnTestEnd(const ::testing::TestInfo& test_info) override {
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - current_test_start_);

        const char* result = test_info.result()->Passed() ? "PASSED" : "FAILED";
        std::cout << "[ " << result << " ] " << test_info.test_suite_name()
                  << "." << test_info.name() << " (" << duration.count() << "ms)" << std::endl;

        if (!test_info.result()->Passed()) {
            for (int i = 0; i < test_info.result()->total_part_count(); ++i) {
                const auto& part = test_info.result()->GetTestPartResult(i);
                if (part.failed()) {
                    std::cout << "    Failure: " << part.summary() << std::endl;
                }
            }
        }
    }

    void OnTestProgramStart(const ::testing::UnitTest& /*unit_test*/) override {
        std::cout << "========================================" << std::endl;
        std::cout << "Workspace Window Enhancement Test Suite" << std::endl;
        std::cout << "========================================" << std::endl;
    }

    void OnTestProgramEnd(const ::testing::UnitTest& unit_test) override {
        std::cout << "========================================" << std::endl;
        std::cout << "Test Summary:" << std::endl;
        std::cout << "  Total tests: " << unit_test.total_test_count() << std::endl;
        std::cout << "  Passed: " << unit_test.successful_test_count() << std::endl;
        std::cout << "  Failed: " << unit_test.failed_test_count() << std::endl;
        std::cout << "  Skipped: " << unit_test.skipped_test_count() << std::endl;

        if (unit_test.failed_test_count() > 0) {
            std::cout << "\nFailed tests:" << std::endl;
            for (int i = 0; i < unit_test.total_test_suite_count(); ++i) {
                const auto* suite = unit_test.GetTestSuite(i);
                for (int j = 0; j < suite->total_test_count(); ++j) {
                    const auto* test = suite->GetTestInfo(j);
                    if (!test->result()->Passed()) {
                        std::cout << "  - " << suite->name() << "." << test->name() << std::endl;
                    }
                }
            }
        }

        std::cout << "========================================" << std::endl;
    }

private:
    std::chrono::steady_clock::time_point current_test_start_;
};

int main(int argc, char** argv) {
    std::cout << "Initializing Workspace Window Enhancement Tests..." << std::endl;

    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);

    // Add our custom environment
    ::testing::AddGlobalTestEnvironment(new WorkspaceTestEnvironment);

    // Add custom test listener
    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();

    // Remove default listener and add our custom one
    delete listeners.Release(listeners.default_result_printer());
    listeners.Append(new WorkspaceTestListener);

    // Set up test filtering based on command line arguments
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--unit") {
            ::testing::GTEST_FLAG(filter) = "*Test.*:*WindowInfo*:*SearchQuery*:*FilterResult*";
            std::cout << "Running unit tests only..." << std::endl;
        } else if (arg == "--integration") {
            ::testing::GTEST_FLAG(filter) = "*Integration*:*Workspace*:*Compatibility*";
            std::cout << "Running integration tests only..." << std::endl;
        } else if (arg == "--performance") {
            ::testing::GTEST_FLAG(filter) = "*Performance*:*Benchmark*";
            std::cout << "Running performance tests only..." << std::endl;
        } else if (arg == "--compatibility") {
            ::testing::GTEST_FLAG(filter) = "*Compatibility*";
            std::cout << "Running compatibility tests only..." << std::endl;
        }
    }

    // Configure test behavior
    ::testing::GTEST_FLAG(break_on_failure) = false;
    ::testing::GTEST_FLAG(catch_exceptions) = true;
    ::testing::GTEST_FLAG(print_time) = true;

    // Run the tests
    int result = RUN_ALL_TESTS();

    if (result == 0) {
        std::cout << "\n🎉 All tests passed! Workspace Window Enhancement implementation is ready." << std::endl;
    } else {
        std::cout << "\n❌ Some tests failed. Please review the implementation." << std::endl;
    }

    return result;
}