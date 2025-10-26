#include <gtest/gtest.h>
#include "../../src/core/compatibility_validator.hpp"
#include "../../src/core/window_manager.hpp"
#include "../../src/filters/search_query.hpp"

namespace WindowManager {
namespace Tests {

class CompatibilityTest : public ::testing::Test {
protected:
    void SetUp() override {
        windowManager = WindowManager::create();

        // Create test windows with both legacy and new fields
        createTestWindows();
    }

    void createTestWindows() {
        // Legacy-style window (minimal fields)
        WindowInfo legacyWindow;
        legacyWindow.handle = "legacy_handle";
        legacyWindow.title = "Legacy Window";
        legacyWindow.x = 100;
        legacyWindow.y = 100;
        legacyWindow.width = 800;
        legacyWindow.height = 600;
        legacyWindow.isVisible = true;
        legacyWindow.processId = 1234;
        legacyWindow.ownerName = "legacy.exe";
        testWindows.push_back(legacyWindow);

        // Enhanced window (all fields)
        WindowInfo enhancedWindow;
        enhancedWindow.handle = "enhanced_handle";
        enhancedWindow.title = "Enhanced Window";
        enhancedWindow.x = 200;
        enhancedWindow.y = 200;
        enhancedWindow.width = 1000;
        enhancedWindow.height = 700;
        enhancedWindow.isVisible = true;
        enhancedWindow.processId = 5678;
        enhancedWindow.ownerName = "enhanced.exe";
        enhancedWindow.workspaceId = "workspace_1";
        enhancedWindow.workspaceName = "Development";
        enhancedWindow.isOnCurrentWorkspace = true;
        enhancedWindow.state = WindowState::Focused;
        enhancedWindow.isFocused = true;
        enhancedWindow.isMinimized = false;
        testWindows.push_back(enhancedWindow);
    }

    std::unique_ptr<WindowManager> windowManager;
    std::vector<WindowInfo> testWindows;
};

TEST_F(CompatibilityTest, LegacyJsonFieldsPresent) {
    for (const auto& window : testWindows) {
        auto result = CompatibilityValidator::validateWindowInfoJson(window);

        EXPECT_TRUE(result.isCompatible) << "Window JSON should be backward compatible";

        if (!result.errors.empty()) {
            for (const auto& error : result.errors) {
                ADD_FAILURE() << "Compatibility error: " << error;
            }
        }
    }
}

TEST_F(CompatibilityTest, JsonSchemaEvolution) {
    auto legacySchema = CompatibilityValidator::getLegacyWindowInfoSchema();
    auto currentSchema = CompatibilityValidator::getCurrentWindowInfoSchema();

    // Current schema should include all legacy fields
    for (const auto& legacyField : legacySchema.requiredFields) {
        EXPECT_NE(currentSchema.requiredFields.find(legacyField), currentSchema.requiredFields.end())
            << "Legacy field '" << legacyField << "' missing from current schema";
    }

    // Current schema should have more fields than legacy
    EXPECT_GT(currentSchema.requiredFields.size(), legacySchema.requiredFields.size())
        << "Current schema should have additional fields";

    // Field types should remain consistent for legacy fields
    for (const auto& legacyField : legacySchema.requiredFields) {
        if (currentSchema.fieldTypes.find(legacyField) != currentSchema.fieldTypes.end()) {
            EXPECT_EQ(legacySchema.fieldTypes.at(legacyField),
                     currentSchema.fieldTypes.at(legacyField))
                << "Field type changed for legacy field: " << legacyField;
        }
    }
}

TEST_F(CompatibilityTest, LegacyConsumerSimulation) {
    for (const auto& window : testWindows) {
        std::string json = window.toJson();
        auto result = CompatibilityValidator::simulateLegacyConsumer(json);

        EXPECT_TRUE(result.isCompatible)
            << "Legacy consumer should be able to parse enhanced JSON";

        if (!result.errors.empty()) {
            for (const auto& error : result.errors) {
                ADD_FAILURE() << "Legacy consumer error: " << error;
            }
        }
    }
}

TEST_F(CompatibilityTest, RequiredLegacyFieldsExtraction) {
    for (const auto& window : testWindows) {
        std::string json = window.toJson();

        EXPECT_TRUE(CompatibilityValidator::hasRequiredLegacyFields(json))
            << "JSON missing required legacy fields";

        // Test field extraction
        auto fields = CompatibilityValidator::extractJsonFields(json);
        auto legacySchema = CompatibilityValidator::getLegacyWindowInfoSchema();

        for (const auto& requiredField : legacySchema.requiredFields) {
            EXPECT_NE(fields.find(requiredField), fields.end())
                << "Missing required legacy field: " << requiredField;
        }
    }
}

TEST_F(CompatibilityTest, FilterResultCompatibility) {
    // Create FilterResult with test data
    SearchQuery query("test");
    auto searchTime = std::chrono::milliseconds(100);
    FilterResult result(testWindows, testWindows.size(), query, searchTime);

    // Test basic JSON output
    std::string basicJson = result.toJson();
    auto basicResult = CompatibilityValidator::validateFilterResultJson(basicJson);

    EXPECT_TRUE(basicResult.isCompatible)
        << "FilterResult basic JSON should be compatible";

    // Test enhanced JSON output with workspaces
    std::vector<WorkspaceInfo> workspaces;
    WorkspaceInfo ws;
    ws.id = "workspace_1";
    ws.name = "Development";
    ws.index = 0;
    ws.isCurrent = true;
    workspaces.push_back(ws);

    FilterResult enhancedResult(testWindows, testWindows.size(), query, searchTime, workspaces);
    std::string enhancedJson = enhancedResult.toJsonWithWorkspaces();
    auto enhancedValidation = CompatibilityValidator::validateFilterResultJson(enhancedJson);

    EXPECT_TRUE(enhancedValidation.isCompatible)
        << "FilterResult enhanced JSON should be compatible";
}

TEST_F(CompatibilityTest, BackwardCompatibilityValidation) {
    auto result = CompatibilityValidator::validateJsonBackwardCompatibility();

    EXPECT_TRUE(result.isCompatible)
        << "Overall backward compatibility validation should pass";

    // Print warnings for information
    for (const auto& warning : result.warnings) {
        std::cout << "Compatibility warning: " << warning << std::endl;
    }

    // Print added fields for information
    if (!result.addedFields.empty()) {
        std::cout << "Added fields: ";
        for (size_t i = 0; i < result.addedFields.size(); ++i) {
            std::cout << result.addedFields[i];
            if (i < result.addedFields.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }

    // Fail on any compatibility errors
    for (const auto& error : result.errors) {
        ADD_FAILURE() << "Compatibility error: " << error;
    }
}

TEST_F(CompatibilityTest, ComprehensiveCompatibilityTest) {
    auto result = CompatibilityTestRunner::runFullCompatibilityTest();

    EXPECT_TRUE(result.isCompatible)
        << "Comprehensive compatibility test should pass";

    // Report any issues
    for (const auto& error : result.errors) {
        ADD_FAILURE() << "Comprehensive test error: " << error;
    }

    for (const auto& warning : result.warnings) {
        std::cout << "Comprehensive test warning: " << warning << std::endl;
    }
}

TEST_F(CompatibilityTest, CompatibilityReportGeneration) {
    std::string report = CompatibilityValidator::generateCompatibilityReport();

    EXPECT_FALSE(report.empty()) << "Compatibility report should not be empty";

    // Should contain key sections
    EXPECT_NE(report.find("Backward Compatibility Validation Report"), std::string::npos);
    EXPECT_NE(report.find("Overall Compatibility"), std::string::npos);
    EXPECT_NE(report.find("SCHEMA EVOLUTION"), std::string::npos);
    EXPECT_NE(report.find("COMPATIBILITY STRATEGY"), std::string::npos);

    std::cout << "Compatibility Report:" << std::endl;
    std::cout << report << std::endl;
}

TEST_F(CompatibilityTest, JsonStructureValidation) {
    for (const auto& window : testWindows) {
        std::string json = window.toJson();

        // Test basic JSON structure
        EXPECT_EQ(json.front(), '{') << "JSON should start with opening brace";
        EXPECT_EQ(json.back(), '}') << "JSON should end with closing brace";

        // Test brace balance
        int braceCount = 0;
        for (char c : json) {
            if (c == '{') braceCount++;
            else if (c == '}') braceCount--;
        }
        EXPECT_EQ(braceCount, 0) << "JSON braces should be balanced";

        // Test that it contains expected JSON patterns
        EXPECT_NE(json.find("\":"), std::string::npos) << "JSON should contain key-value separators";
        EXPECT_NE(json.find("\""), std::string::npos) << "JSON should contain quoted strings";
    }
}

TEST_F(CompatibilityTest, NewFieldsAreAdditive) {
    // Test that new fields don't interfere with legacy parsing

    WindowInfo legacyStyle;
    legacyStyle.handle = "legacy";
    legacyStyle.title = "Legacy Window";
    legacyStyle.x = 0;
    legacyStyle.y = 0;
    legacyStyle.width = 800;
    legacyStyle.height = 600;
    legacyStyle.isVisible = true;
    legacyStyle.processId = 1000;
    legacyStyle.ownerName = "legacy.exe";
    // No workspace fields set

    std::string legacyJson = legacyStyle.toJson();

    // Should still contain workspace fields (with default values)
    EXPECT_NE(legacyJson.find("\"workspaceId\""), std::string::npos);
    EXPECT_NE(legacyJson.find("\"state\""), std::string::npos);
    EXPECT_NE(legacyJson.find("\"isFocused\""), std::string::npos);

    // But legacy fields should still be present and parseable
    EXPECT_NE(legacyJson.find("\"handle\""), std::string::npos);
    EXPECT_NE(legacyJson.find("\"legacy\""), std::string::npos);

    auto validation = CompatibilityValidator::validateWindowInfoJson(legacyStyle);
    EXPECT_TRUE(validation.isCompatible);
}

TEST_F(CompatibilityTest, PerformanceRegressionTest) {
    // Ensure new features don't significantly impact performance

    auto start = std::chrono::steady_clock::now();

    // Test JSON generation performance
    for (int i = 0; i < 1000; ++i) {
        for (const auto& window : testWindows) {
            std::string json = window.toJson();
            (void)json; // Suppress unused variable warning
        }
    }

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);

    // Should generate 2000 JSON outputs in reasonable time (<1 second)
    EXPECT_LT(elapsed.count(), 1000)
        << "JSON generation performance regression detected";

    std::cout << "JSON generation performance: " << elapsed.count()
              << "ms for 2000 operations" << std::endl;
}

TEST_F(CompatibilityTest, CrossPlatformCompatibility) {
    // Test that JSON output is consistent across platforms

    for (const auto& window : testWindows) {
        std::string json = window.toJson();

        // Should not contain platform-specific formatting
        EXPECT_EQ(json.find('\r'), std::string::npos)
            << "JSON should not contain carriage returns";

        // Should use standard JSON formatting
        EXPECT_NE(json.find('\n'), std::string::npos)
            << "JSON should contain proper line breaks";

        // Field names should be consistent
        auto fields = CompatibilityValidator::extractJsonFields(json);

        // Key fields should use standard naming
        EXPECT_NE(fields.find("processId"), fields.end())
            << "Should use camelCase for processId";
        EXPECT_NE(fields.find("workspaceId"), fields.end())
            << "Should use camelCase for workspaceId";
        EXPECT_NE(fields.find("isVisible"), fields.end())
            << "Should use camelCase for isVisible";
    }
}

} // namespace Tests
} // namespace WindowManager