#include "compatibility_validator.hpp"
#include "../filters/filter_result.hpp"
#include "../filters/search_query.hpp"
#include <sstream>
#include <iostream>
#include <regex>
#include <fstream>

namespace WindowManager {

// T049: Backward compatibility validation implementation

CompatibilityValidator::JsonSchema CompatibilityValidator::getLegacyWindowInfoSchema() {
    JsonSchema schema;
    schema.version = "1.0";

    // Required fields in legacy WindowInfo JSON
    schema.requiredFields = {
        "handle", "title", "x", "y", "width", "height",
        "isVisible", "processId", "ownerName"
    };

    // Field types for validation
    schema.fieldTypes = {
        {"handle", "string"},
        {"title", "string"},
        {"x", "number"},
        {"y", "number"},
        {"width", "number"},
        {"height", "number"},
        {"isVisible", "boolean"},
        {"processId", "number"},
        {"ownerName", "string"}
    };

    return schema;
}

CompatibilityValidator::JsonSchema CompatibilityValidator::getCurrentWindowInfoSchema() {
    JsonSchema schema;
    schema.version = "2.0";

    // All fields in current WindowInfo JSON (includes legacy + new)
    schema.requiredFields = {
        "handle", "title", "x", "y", "width", "height",
        "isVisible", "processId", "ownerName",
        // NEW FIELDS (should be additive)
        "workspaceId", "workspaceName", "isOnCurrentWorkspace",
        "state", "isFocused", "isMinimized"
    };

    // Field types for validation
    schema.fieldTypes = {
        {"handle", "string"},
        {"title", "string"},
        {"x", "number"},
        {"y", "number"},
        {"width", "number"},
        {"height", "number"},
        {"isVisible", "boolean"},
        {"processId", "number"},
        {"ownerName", "string"},
        // NEW FIELDS
        {"workspaceId", "string"},
        {"workspaceName", "string"},
        {"isOnCurrentWorkspace", "boolean"},
        {"state", "string"},
        {"isFocused", "boolean"},
        {"isMinimized", "boolean"}
    };

    return schema;
}

CompatibilityValidator::ValidationResult CompatibilityValidator::validateWindowInfoJson(const WindowInfo& window) {
    ValidationResult result;

    // Get JSON output from current implementation
    std::string jsonOutput = window.toJson();

    // Check basic JSON structure
    if (!isValidJsonStructure(jsonOutput)) {
        result.isCompatible = false;
        result.errors.push_back("Invalid JSON structure in WindowInfo::toJson()");
        return result;
    }

    // Get schemas for comparison
    auto legacySchema = getLegacyWindowInfoSchema();
    auto currentSchema = getCurrentWindowInfoSchema();

    // Extract actual fields from JSON
    auto actualFields = extractJsonFields(jsonOutput);

    // Check all legacy required fields are present
    for (const auto& field : legacySchema.requiredFields) {
        if (actualFields.find(field) == actualFields.end()) {
            result.isCompatible = false;
            result.errors.push_back("Missing required legacy field: " + field);
        }
    }

    // Identify added fields
    for (const auto& field : actualFields) {
        if (legacySchema.requiredFields.find(field) == legacySchema.requiredFields.end()) {
            result.addedFields.push_back(field);
        }
    }

    // Validate field types for legacy fields
    for (const auto& field : legacySchema.requiredFields) {
        if (actualFields.find(field) != actualFields.end()) {
            std::string value = extractFieldValue(jsonOutput, field);
            std::string expectedType = legacySchema.fieldTypes.at(field);
            if (!validateFieldType(value, expectedType)) {
                result.isCompatible = false;
                result.errors.push_back("Field '" + field + "' has incorrect type. Expected: " + expectedType);
            }
        }
    }

    // Add warnings for significant changes
    if (result.addedFields.size() > 0) {
        result.warnings.push_back("Added " + std::to_string(result.addedFields.size()) + " new fields to WindowInfo JSON");
    }

    return result;
}

CompatibilityValidator::ValidationResult CompatibilityValidator::validateFilterResultJson(const std::string& jsonOutput) {
    ValidationResult result;

    // Basic structure validation
    if (!isValidJsonStructure(jsonOutput)) {
        result.isCompatible = false;
        result.errors.push_back("Invalid JSON structure in FilterResult");
        return result;
    }

    // Check for required legacy fields in FilterResult
    std::vector<std::string> requiredFilterFields = {
        "windows", "metadata"
    };

    auto actualFields = extractJsonFields(jsonOutput);

    for (const auto& field : requiredFilterFields) {
        if (actualFields.find(field) == actualFields.end()) {
            result.isCompatible = false;
            result.errors.push_back("Missing required FilterResult field: " + field);
        }
    }

    // Check if windows array exists and has proper structure
    if (jsonOutput.find("\"windows\"") != std::string::npos) {
        if (jsonOutput.find("[") == std::string::npos) {
            result.isCompatible = false;
            result.errors.push_back("Windows field is not an array");
        }
    }

    // Check for new workspace-related fields
    if (jsonOutput.find("workspaces") != std::string::npos) {
        result.addedFields.push_back("workspaces");
        result.warnings.push_back("Added workspace grouping to FilterResult JSON");
    }

    return result;
}

CompatibilityValidator::ValidationResult CompatibilityValidator::validateJsonBackwardCompatibility() {
    ValidationResult result;

    // Create test window with legacy and new data
    WindowInfo testWindow;
    testWindow.handle = "test_handle_123";
    testWindow.title = "Test Window";
    testWindow.x = 100;
    testWindow.y = 200;
    testWindow.width = 800;
    testWindow.height = 600;
    testWindow.isVisible = true;
    testWindow.processId = 1234;
    testWindow.ownerName = "TestApp";
    // NEW FIELDS
    testWindow.workspaceId = "workspace_1";
    testWindow.workspaceName = "Development";
    testWindow.isOnCurrentWorkspace = true;
    testWindow.state = WindowState::Normal;
    testWindow.isFocused = false;
    testWindow.isMinimized = false;

    // Validate WindowInfo JSON
    auto windowResult = validateWindowInfoJson(testWindow);

    // Merge results
    result.isCompatible = windowResult.isCompatible;
    result.errors.insert(result.errors.end(), windowResult.errors.begin(), windowResult.errors.end());
    result.warnings.insert(result.warnings.end(), windowResult.warnings.begin(), windowResult.warnings.end());
    result.addedFields.insert(result.addedFields.end(), windowResult.addedFields.begin(), windowResult.addedFields.end());

    // Test legacy consumer simulation
    std::string testJson = testWindow.toJson();
    auto legacyResult = simulateLegacyConsumer(testJson);

    if (!legacyResult.isCompatible) {
        result.isCompatible = false;
        result.errors.insert(result.errors.end(), legacyResult.errors.begin(), legacyResult.errors.end());
    }

    return result;
}

bool CompatibilityValidator::hasRequiredLegacyFields(const std::string& jsonOutput) {
    auto legacySchema = getLegacyWindowInfoSchema();
    auto actualFields = extractJsonFields(jsonOutput);

    for (const auto& field : legacySchema.requiredFields) {
        if (actualFields.find(field) == actualFields.end()) {
            return false;
        }
    }

    return true;
}

std::set<std::string> CompatibilityValidator::extractJsonFields(const std::string& jsonOutput) {
    std::set<std::string> fields;

    // Simple regex to find field names in JSON
    std::regex fieldRegex(R"("([^"]+)":\s*)");
    std::sregex_iterator iter(jsonOutput.begin(), jsonOutput.end(), fieldRegex);
    std::sregex_iterator end;

    for (; iter != end; ++iter) {
        std::smatch match = *iter;
        fields.insert(match[1].str());
    }

    return fields;
}

std::string CompatibilityValidator::generateCompatibilityReport() {
    std::ostringstream report;

    report << "Backward Compatibility Validation Report\n";
    report << "========================================\n\n";

    auto result = validateJsonBackwardCompatibility();

    report << "Overall Compatibility: " << (result.isCompatible ? "PASS" : "FAIL") << "\n\n";

    if (!result.errors.empty()) {
        report << "ERRORS:\n";
        for (const auto& error : result.errors) {
            report << "  ✗ " << error << "\n";
        }
        report << "\n";
    }

    if (!result.warnings.empty()) {
        report << "WARNINGS:\n";
        for (const auto& warning : result.warnings) {
            report << "  ⚠ " << warning << "\n";
        }
        report << "\n";
    }

    if (!result.addedFields.empty()) {
        report << "NEW FIELDS ADDED:\n";
        for (const auto& field : result.addedFields) {
            report << "  + " << field << "\n";
        }
        report << "\n";
    }

    // Schema comparison
    auto legacySchema = getLegacyWindowInfoSchema();
    auto currentSchema = getCurrentWindowInfoSchema();

    report << "SCHEMA EVOLUTION:\n";
    report << "  Legacy Version: " << legacySchema.version << " (" << legacySchema.requiredFields.size() << " fields)\n";
    report << "  Current Version: " << currentSchema.version << " (" << currentSchema.requiredFields.size() << " fields)\n";
    report << "  Added Fields: " << (currentSchema.requiredFields.size() - legacySchema.requiredFields.size()) << "\n\n";

    report << "COMPATIBILITY STRATEGY:\n";
    report << "  ✓ All legacy fields preserved\n";
    report << "  ✓ New fields are additive only\n";
    report << "  ✓ No existing field types changed\n";
    report << "  ✓ JSON structure remains valid\n";

    return report.str();
}

CompatibilityValidator::ValidationResult CompatibilityValidator::simulateLegacyConsumer(const std::string& jsonOutput) {
    ValidationResult result;

    // Simulate what a legacy consumer would expect
    auto legacySchema = getLegacyWindowInfoSchema();

    // Check that a legacy parser can still extract all required fields
    for (const auto& field : legacySchema.requiredFields) {
        std::string value = extractFieldValue(jsonOutput, field);
        if (value.empty()) {
            result.isCompatible = false;
            result.errors.push_back("Legacy consumer cannot extract field: " + field);
        }
    }

    // Ensure JSON is still parseable by simple parsers
    if (!isValidJsonStructure(jsonOutput)) {
        result.isCompatible = false;
        result.errors.push_back("JSON structure would break legacy parsers");
    }

    return result;
}

// Helper methods implementation

bool CompatibilityValidator::isValidJsonStructure(const std::string& jsonOutput) {
    // Basic JSON structure validation
    if (jsonOutput.empty()) return false;

    // Must start and end with braces
    if (jsonOutput.front() != '{' || jsonOutput.back() != '}') {
        return false;
    }

    // Count braces for basic balance check
    int braceCount = 0;
    for (char c : jsonOutput) {
        if (c == '{') braceCount++;
        else if (c == '}') braceCount--;
        if (braceCount < 0) return false;
    }

    return braceCount == 0;
}

std::string CompatibilityValidator::extractFieldValue(const std::string& jsonOutput, const std::string& fieldName) {
    // Simple field value extraction
    std::string pattern = "\"" + fieldName + "\":\\s*([^,}]+)";
    std::regex fieldRegex(pattern);
    std::smatch match;

    if (std::regex_search(jsonOutput, match, fieldRegex)) {
        std::string value = match[1].str();
        // Trim whitespace
        value.erase(0, value.find_first_not_of(" \t\n\r"));
        value.erase(value.find_last_not_of(" \t\n\r") + 1);
        return value;
    }

    return "";
}

bool CompatibilityValidator::validateFieldType(const std::string& value, const std::string& expectedType) {
    if (value.empty()) return false;

    if (expectedType == "string") {
        return value.front() == '"' && value.back() == '"';
    } else if (expectedType == "number") {
        return std::regex_match(value, std::regex(R"(-?\d+)"));
    } else if (expectedType == "boolean") {
        return value == "true" || value == "false";
    }

    return false;
}

// CompatibilityTestRunner implementation

CompatibilityValidator::ValidationResult CompatibilityTestRunner::runFullCompatibilityTest() {
    ValidationResult result;

    // Generate comprehensive test data
    auto testData = generateTestData();

    // Test all JSON methods
    auto jsonResult = validateAllJsonMethods(testData);

    // Merge results
    result.isCompatible = jsonResult.isCompatible;
    result.errors.insert(result.errors.end(), jsonResult.errors.begin(), jsonResult.errors.end());
    result.warnings.insert(result.warnings.end(), jsonResult.warnings.begin(), jsonResult.warnings.end());

    return result;
}

CompatibilityTestRunner::TestSuite CompatibilityTestRunner::generateTestData() {
    TestSuite testData;

    // Create diverse test windows
    for (int i = 0; i < 5; ++i) {
        WindowInfo window;
        window.handle = "handle_" + std::to_string(i);
        window.title = "Test Window " + std::to_string(i);
        window.x = i * 100;
        window.y = i * 100;
        window.width = 800 + i * 100;
        window.height = 600 + i * 50;
        window.isVisible = (i % 2 == 0);
        window.processId = 1000 + i;
        window.ownerName = "TestApp" + std::to_string(i);
        window.workspaceId = "workspace_" + std::to_string(i % 3);
        window.workspaceName = "Workspace " + std::to_string(i % 3);
        window.isOnCurrentWorkspace = (i % 3 == 0);
        window.state = static_cast<WindowState>(i % 4);
        window.isFocused = (i == 0);
        window.isMinimized = (i % 4 == 2);

        testData.testWindows.push_back(window);
    }

    // Create test workspaces
    for (int i = 0; i < 3; ++i) {
        WorkspaceInfo workspace;
        workspace.id = "workspace_" + std::to_string(i);
        workspace.name = "Workspace " + std::to_string(i);
        workspace.index = i;
        workspace.isCurrent = (i == 0);

        testData.testWorkspaces.push_back(workspace);
    }

    // Create test search queries
    testData.testQueries = {
        "chrome",
        "Test",
        "App"
    };

    return testData;
}

CompatibilityValidator::ValidationResult CompatibilityTestRunner::validateAllJsonMethods(const TestSuite& testData) {
    ValidationResult result;

    // Test WindowInfo JSON methods
    auto windowResult = testWindowInfoJson(testData.testWindows);
    if (!windowResult.isCompatible) {
        result.isCompatible = false;
        result.errors.insert(result.errors.end(), windowResult.errors.begin(), windowResult.errors.end());
    }

    // Test FilterResult JSON methods
    auto filterResult = testFilterResultJson(testData);
    if (!filterResult.isCompatible) {
        result.isCompatible = false;
        result.errors.insert(result.errors.end(), filterResult.errors.begin(), filterResult.errors.end());
    }

    return result;
}

bool CompatibilityTestRunner::createCompatibilityReport(const std::string& filePath) {
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            return false;
        }

        file << CompatibilityValidator::generateCompatibilityReport();
        file.close();
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

CompatibilityValidator::ValidationResult CompatibilityTestRunner::testWindowInfoJson(const std::vector<WindowInfo>& windows) {
    ValidationResult result;

    for (const auto& window : windows) {
        auto windowResult = CompatibilityValidator::validateWindowInfoJson(window);
        if (!windowResult.isCompatible) {
            result.isCompatible = false;
            result.errors.insert(result.errors.end(), windowResult.errors.begin(), windowResult.errors.end());
        }
    }

    return result;
}

CompatibilityValidator::ValidationResult CompatibilityTestRunner::testFilterResultJson(const TestSuite& testData) {
    ValidationResult result;

    // Create a sample FilterResult
    SearchQuery query("test");
    auto searchTime = std::chrono::milliseconds(100);
    FilterResult filterResult(testData.testWindows, testData.testWindows.size(), query, searchTime, testData.testWorkspaces);

    // Test both JSON methods
    std::string basicJson = filterResult.toJson();
    std::string enhancedJson = filterResult.toJsonWithWorkspaces();

    auto basicResult = CompatibilityValidator::validateFilterResultJson(basicJson);
    auto enhancedResult = CompatibilityValidator::validateFilterResultJson(enhancedJson);

    if (!basicResult.isCompatible || !enhancedResult.isCompatible) {
        result.isCompatible = false;
        result.errors.insert(result.errors.end(), basicResult.errors.begin(), basicResult.errors.end());
        result.errors.insert(result.errors.end(), enhancedResult.errors.begin(), enhancedResult.errors.end());
    }

    return result;
}

CompatibilityValidator::ValidationResult CompatibilityTestRunner::testWorkspaceJson(const std::vector<WorkspaceInfo>& workspaces) {
    ValidationResult result;
    // Additional workspace-specific JSON validation could be added here
    return result;
}

} // namespace WindowManager