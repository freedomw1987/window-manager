#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include "window.hpp"
#include "workspace.hpp"

namespace WindowManager {

/**
 * T049: Backward compatibility validation for JSON output formats
 * Ensures new fields are additive and don't break existing consumers
 */
class CompatibilityValidator {
public:
    struct ValidationResult {
        bool isCompatible = true;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
        std::vector<std::string> addedFields;
        std::vector<std::string> modifiedFields;
        std::vector<std::string> removedFields;
    };

    struct JsonSchema {
        std::set<std::string> requiredFields;
        std::map<std::string, std::string> fieldTypes;
        std::string version;
    };

    // Legacy JSON schema for WindowInfo (v1.0)
    static JsonSchema getLegacyWindowInfoSchema();

    // Current JSON schema for WindowInfo (v2.0)
    static JsonSchema getCurrentWindowInfoSchema();

    // Validate individual WindowInfo JSON output
    static ValidationResult validateWindowInfoJson(const WindowInfo& window);

    // Validate FilterResult JSON output compatibility
    static ValidationResult validateFilterResultJson(const std::string& jsonOutput);

    // Validate enhanced JSON features don't break basic consumers
    static ValidationResult validateJsonBackwardCompatibility();

    // Check if JSON contains all required legacy fields
    static bool hasRequiredLegacyFields(const std::string& jsonOutput);

    // Extract field names from JSON string (simple parser for validation)
    static std::set<std::string> extractJsonFields(const std::string& jsonOutput);

    // Generate compatibility report
    static std::string generateCompatibilityReport();

    // Test JSON parsing with legacy consumers (simulation)
    static ValidationResult simulateLegacyConsumer(const std::string& jsonOutput);

private:
    // Helper methods for JSON validation
    static bool isValidJsonStructure(const std::string& jsonOutput);
    static std::string extractFieldValue(const std::string& jsonOutput, const std::string& fieldName);
    static bool validateFieldType(const std::string& value, const std::string& expectedType);
};

/**
 * Compatibility test runner for comprehensive validation
 */
class CompatibilityTestRunner {
public:
    struct TestSuite {
        std::vector<WindowInfo> testWindows;
        std::vector<WorkspaceInfo> testWorkspaces;
        std::vector<std::string> testQueries;
    };

    // Run comprehensive backward compatibility tests
    static ValidationResult runFullCompatibilityTest();

    // Generate test data for validation
    static TestSuite generateTestData();

    // Validate all JSON output methods
    static ValidationResult validateAllJsonMethods(const TestSuite& testData);

    // Create compatibility report file
    static bool createCompatibilityReport(const std::string& filePath);

private:
    static ValidationResult testWindowInfoJson(const std::vector<WindowInfo>& windows);
    static ValidationResult testFilterResultJson(const TestSuite& testData);
    static ValidationResult testWorkspaceJson(const std::vector<WorkspaceInfo>& workspaces);
};

} // namespace WindowManager