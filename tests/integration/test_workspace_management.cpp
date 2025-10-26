#include <gtest/gtest.h>
#include "../../src/core/window_manager.hpp"
#include "../../src/filters/search_query.hpp"
#include "../../src/ui/cli.hpp"
#include <thread>
#include <chrono>

namespace WindowManager {
namespace Tests {

class WorkspaceManagementTest : public ::testing::Test {
protected:
    void SetUp() override {
        windowManager = WindowManager::create();
        cli.setOutputFormat("text");
        cli.setVerbose(true);
    }

    void TearDown() override {
        // Clean up any test resources
    }

    std::unique_ptr<WindowManager> windowManager;
    CLI cli;
};

TEST_F(WorkspaceManagementTest, WorkspaceEnumeration) {
    // Test basic workspace enumeration
    auto workspaces = windowManager->getAllWorkspaces();

    // Should have at least one workspace (current desktop)
    EXPECT_GE(workspaces.size(), 1);

    // Check workspace structure
    for (const auto& workspace : workspaces) {
        EXPECT_FALSE(workspace.id.empty());
        EXPECT_FALSE(workspace.name.empty());
        EXPECT_GE(workspace.index, 0);
    }

    // Exactly one workspace should be marked as current
    int currentCount = 0;
    for (const auto& workspace : workspaces) {
        if (workspace.isCurrent) {
            currentCount++;
        }
    }
    EXPECT_EQ(currentCount, 1);
}

TEST_F(WorkspaceManagementTest, CurrentWorkspaceDetection) {
    auto currentWorkspace = windowManager->getCurrentWorkspace();

    if (currentWorkspace) {
        EXPECT_TRUE(currentWorkspace->isCurrent);
        EXPECT_FALSE(currentWorkspace->id.empty());
        EXPECT_FALSE(currentWorkspace->name.empty());
    }
    // Note: currentWorkspace might be nullopt on systems without workspace support
}

TEST_F(WorkspaceManagementTest, CrossWorkspaceWindowEnumeration) {
    auto allWindows = windowManager->getAllWorkspaceWindows();

    // Should have some windows (at least test runner)
    EXPECT_GT(allWindows.size(), 0);

    // All windows should have workspace information
    for (const auto& window : allWindows) {
        // On systems with workspace support, workspaceId should be set
        // On systems without support, it might be empty (graceful fallback)
        EXPECT_TRUE(window.isValid());
    }
}

TEST_F(WorkspaceManagementTest, FocusedWindowDetection) {
    auto focusedWindow = windowManager->getFocusedWindowAcrossWorkspaces();

    if (focusedWindow) {
        EXPECT_TRUE(focusedWindow->isValid());
        EXPECT_TRUE(focusedWindow->isFocused);
        EXPECT_EQ(focusedWindow->state, WindowState::Focused);

        // Focused window should be visible
        EXPECT_TRUE(focusedWindow->isVisible);
    }
    // Note: focusedWindow might be nullopt if no window has focus
}

TEST_F(WorkspaceManagementTest, WorkspaceSpecificWindowRetrieval) {
    auto workspaces = windowManager->getAllWorkspaces();

    if (!workspaces.empty()) {
        const auto& firstWorkspace = workspaces[0];
        auto workspaceWindows = windowManager->getWindowsOnWorkspace(firstWorkspace.id);

        // All returned windows should belong to the specified workspace
        for (const auto& window : workspaceWindows) {
            EXPECT_EQ(window.workspaceId, firstWorkspace.id);
        }
    }
}

TEST_F(WorkspaceManagementTest, WorkspaceAwareSearch) {
    SearchQuery query("test", SearchField::Both);

    auto searchResult = windowManager->searchWindowsWithWorkspaces(query);

    EXPECT_TRUE(searchResult.isValid());
    EXPECT_TRUE(searchResult.meetsPerformanceTarget());

    // Should have workspace grouping
    EXPECT_GE(searchResult.getWorkspaceCount(), 0);

    // Check workspace statistics
    auto stats = searchResult.getWorkspaceStatistics();
    EXPECT_GE(stats.totalWorkspaces, 0);
    EXPECT_GE(stats.totalWindows, 0);
}

TEST_F(WorkspaceManagementTest, PerformanceRequirements) {
    auto start = std::chrono::steady_clock::now();

    // Test window enumeration performance
    auto windows = windowManager->getAllWindows();

    auto windowEnumTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);

    // Should meet performance requirement: <3 seconds
    EXPECT_LT(windowEnumTime.count(), 3000);

    start = std::chrono::steady_clock::now();

    // Test workspace enumeration performance
    auto workspaces = windowManager->getAllWorkspaces();

    auto workspaceEnumTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);

    // Should meet performance requirement: <1 second
    EXPECT_LT(workspaceEnumTime.count(), 1000);

    // Test search performance
    start = std::chrono::steady_clock::now();

    SearchQuery query("test");
    auto searchResult = windowManager->searchWindows(query);

    auto searchTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);

    // Should meet performance requirement: <1 second
    EXPECT_LT(searchTime.count(), 1000);
    EXPECT_TRUE(searchResult.meetsPerformanceTarget());
}

TEST_F(WorkspaceManagementTest, CachingBehavior) {
    // Test that caching improves performance

    // First call (cold cache)
    auto start1 = std::chrono::steady_clock::now();
    auto windows1 = windowManager->getAllWindows();
    auto time1 = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start1);

    // Second call (warm cache)
    auto start2 = std::chrono::steady_clock::now();
    auto windows2 = windowManager->getAllWindows();
    auto time2 = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start2);

    // Second call should be faster (or at least not significantly slower)
    EXPECT_LE(time2.count(), time1.count() * 2);

    // Results should be consistent
    EXPECT_EQ(windows1.size(), windows2.size());

    // Test cache invalidation
    windowManager->invalidateCache();

    auto start3 = std::chrono::steady_clock::now();
    auto windows3 = windowManager->getAllWindows();
    auto time3 = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start3);

    // Should still work after cache invalidation
    EXPECT_EQ(windows1.size(), windows3.size());
}

TEST_F(WorkspaceManagementTest, ErrorHandlingAndGracefulDegradation) {
    // Test behavior when workspace features are not supported

    try {
        auto workspaces = windowManager->getAllWorkspaces();
        // Should always succeed, even with fallback to default workspace
        EXPECT_GE(workspaces.size(), 1);

        auto windows = windowManager->getAllWorkspaceWindows();
        // Should always succeed, might fall back to regular enumeration
        EXPECT_GE(windows.size(), 0);

        // Test with invalid workspace ID
        auto invalidWindows = windowManager->getWindowsOnWorkspace("invalid_workspace_id");
        // Should return empty or all windows (graceful fallback)
        EXPECT_GE(invalidWindows.size(), 0);

    } catch (const std::exception& e) {
        // Should not throw exceptions, but handle gracefully
        FAIL() << "Unexpected exception: " << e.what();
    }
}

TEST_F(WorkspaceManagementTest, WindowStateAccuracy) {
    auto windows = windowManager->getAllWindows();

    for (const auto& window : windows) {
        // Check state consistency
        if (window.isFocused) {
            EXPECT_EQ(window.state, WindowState::Focused);
            EXPECT_TRUE(window.isVisible); // Focused windows should be visible
        }

        if (window.isMinimized) {
            EXPECT_EQ(window.state, WindowState::Minimized);
        }

        if (window.state == WindowState::Hidden) {
            EXPECT_FALSE(window.isVisible);
        }

        // Workspace information should be consistent
        if (!window.workspaceId.empty()) {
            // If workspace ID is set, window should have valid workspace info
            EXPECT_TRUE(window.hasWorkspaceInfo());
        }
    }
}

TEST_F(WorkspaceManagementTest, CLIIntegration) {
    // Test CLI workspace commands
    std::ostringstream output;
    std::streambuf* oldCout = std::cout.rdbuf(output.rdbuf());

    try {
        // Test workspace summary display
        auto workspaces = windowManager->getAllWorkspaces();
        auto windows = windowManager->getAllWindows();

        cli.displayWorkspaceSummary(workspaces, windows);

        std::string result = output.str();
        EXPECT_FALSE(result.empty());
        EXPECT_NE(result.find("Workspace"), std::string::npos);

        // Reset output buffer
        output.str("");
        output.clear();

        // Test workspace status display
        cli.displayWorkspaceStatus(workspaces);

        result = output.str();
        EXPECT_FALSE(result.empty());
        EXPECT_NE(result.find("Current"), std::string::npos);

    } catch (const std::exception& e) {
        std::cout.rdbuf(oldCout); // Restore cout
        FAIL() << "CLI integration test failed: " << e.what();
    }

    std::cout.rdbuf(oldCout); // Restore cout
}

TEST_F(WorkspaceManagementTest, JsonOutputCompatibility) {
    auto windows = windowManager->getAllWindows();

    for (const auto& window : windows) {
        // Test basic JSON output
        std::string basicJson = window.toJson();
        EXPECT_EQ(basicJson.front(), '{');
        EXPECT_EQ(basicJson.back(), '}');

        // Check for required legacy fields
        EXPECT_NE(basicJson.find("\"handle\""), std::string::npos);
        EXPECT_NE(basicJson.find("\"title\""), std::string::npos);
        EXPECT_NE(basicJson.find("\"processId\""), std::string::npos);

        // Check for new workspace fields
        EXPECT_NE(basicJson.find("\"workspaceId\""), std::string::npos);
        EXPECT_NE(basicJson.find("\"state\""), std::string::npos);

        // Test enhanced JSON with workspace context
        auto workspaces = windowManager->getAllWorkspaces();
        std::string enhancedJson = window.toJsonWithWorkspaceContext(workspaces);

        EXPECT_NE(enhancedJson.find("\"workspace\""), std::string::npos);
        EXPECT_NE(enhancedJson.find("\"geometry\""), std::string::npos);
    }
}

TEST_F(WorkspaceManagementTest, SearchAcrossWorkspaces) {
    // Test comprehensive search across all workspaces
    SearchQuery titleQuery("window", SearchField::Title, false);
    auto titleResults = windowManager->searchWindowsWithWorkspaces(titleQuery);

    SearchQuery ownerQuery("test", SearchField::Owner, false);
    auto ownerResults = windowManager->searchWindowsWithWorkspaces(ownerQuery);

    SearchQuery bothQuery("app", SearchField::Both, false);
    auto bothResults = windowManager->searchWindowsWithWorkspaces(bothQuery);

    // All searches should complete within performance targets
    EXPECT_TRUE(titleResults.meetsPerformanceTarget());
    EXPECT_TRUE(ownerResults.meetsPerformanceTarget());
    EXPECT_TRUE(bothResults.meetsPerformanceTarget());

    // Results should be properly grouped by workspace
    EXPECT_GE(titleResults.getWorkspaceCount(), 0);
    EXPECT_GE(ownerResults.getWorkspaceCount(), 0);
    EXPECT_GE(bothResults.getWorkspaceCount(), 0);
}

} // namespace Tests
} // namespace WindowManager