#include <gtest/gtest.h>
#include "../../src/filters/filter_result.hpp"
#include "../../src/filters/search_query.hpp"
#include "../../src/core/workspace.hpp"

namespace WindowManager {
namespace Tests {

class FilterResultTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test windows
        for (int i = 0; i < 5; ++i) {
            WindowInfo window;
            window.handle = "handle_" + std::to_string(i);
            window.title = "Window " + std::to_string(i);
            window.ownerName = "App" + std::to_string(i);
            window.processId = 1000 + i;
            window.workspaceId = "workspace_" + std::to_string(i % 3);
            window.isVisible = (i % 2 == 0);
            window.isFocused = (i == 0);
            window.isMinimized = (i % 3 == 2);
            window.x = i * 100;
            window.y = i * 100;
            window.width = 800;
            window.height = 600;
            testWindows.push_back(window);
        }

        // Create test workspaces
        for (int i = 0; i < 3; ++i) {
            WorkspaceInfo workspace;
            workspace.id = "workspace_" + std::to_string(i);
            workspace.name = "Workspace " + std::to_string(i);
            workspace.index = i;
            workspace.isCurrent = (i == 0);
            testWorkspaces.push_back(workspace);
        }

        testQuery = SearchQuery("Window", SearchField::Title);
        testTime = std::chrono::milliseconds(150);
    }

    std::vector<WindowInfo> testWindows;
    std::vector<WorkspaceInfo> testWorkspaces;
    SearchQuery testQuery;
    std::chrono::milliseconds testTime;
};

TEST_F(FilterResultTest, BasicConstructor) {
    FilterResult result(testWindows, 10, testQuery, testTime);

    EXPECT_EQ(result.windows.size(), 5);
    EXPECT_EQ(result.totalCount, 10);
    EXPECT_EQ(result.filteredCount, 5);
    EXPECT_EQ(result.searchTime, testTime);
    EXPECT_EQ(result.query.query, "Window");
}

TEST_F(FilterResultTest, WorkspaceConstructor) {
    FilterResult result(testWindows, 10, testQuery, testTime, testWorkspaces);

    EXPECT_EQ(result.windows.size(), 5);
    EXPECT_EQ(result.workspaces.size(), 3);
    EXPECT_FALSE(result.windowsByWorkspace.empty());
}

TEST_F(FilterResultTest, FilterRatioCalculation) {
    FilterResult result(testWindows, 10, testQuery, testTime);

    EXPECT_DOUBLE_EQ(result.filterRatio(), 0.5); // 5 filtered out of 10 total

    // Test edge case - no windows
    FilterResult emptyResult({}, 0, testQuery, testTime);
    EXPECT_DOUBLE_EQ(emptyResult.filterRatio(), 1.0);
}

TEST_F(FilterResultTest, PerformanceTargetValidation) {
    // Fast result
    FilterResult fastResult(testWindows, 10, testQuery, std::chrono::milliseconds(500));
    EXPECT_TRUE(fastResult.meetsPerformanceTarget());

    // Slow result
    FilterResult slowResult(testWindows, 10, testQuery, std::chrono::milliseconds(1500));
    EXPECT_FALSE(slowResult.meetsPerformanceTarget());
}

TEST_F(FilterResultTest, WorkspaceGrouping) {
    FilterResult result(testWindows, 10, testQuery, testTime, testWorkspaces);

    // Should have 3 workspaces in grouping
    EXPECT_EQ(result.getWorkspaceCount(), 3);

    // Check specific workspace counts
    EXPECT_EQ(result.getWindowCountForWorkspace("workspace_0"), 2); // windows 0, 3
    EXPECT_EQ(result.getWindowCountForWorkspace("workspace_1"), 2); // windows 1, 4
    EXPECT_EQ(result.getWindowCountForWorkspace("workspace_2"), 1); // window 2
}

TEST_F(FilterResultTest, WorkspaceIds) {
    FilterResult result(testWindows, 10, testQuery, testTime, testWorkspaces);

    auto workspaceIds = result.getWorkspaceIds();
    EXPECT_EQ(workspaceIds.size(), 3);
    EXPECT_TRUE(std::find(workspaceIds.begin(), workspaceIds.end(), "workspace_0") != workspaceIds.end());
    EXPECT_TRUE(std::find(workspaceIds.begin(), workspaceIds.end(), "workspace_1") != workspaceIds.end());
    EXPECT_TRUE(std::find(workspaceIds.begin(), workspaceIds.end(), "workspace_2") != workspaceIds.end());
}

TEST_F(FilterResultTest, WorkspaceStatsSummary) {
    FilterResult result(testWindows, 10, testQuery, testTime, testWorkspaces);

    std::string summary = result.getWorkspaceStatsSummary();

    EXPECT_NE(summary.find("Workspace 0"), std::string::npos);
    EXPECT_NE(summary.find("Workspace 1"), std::string::npos);
    EXPECT_NE(summary.find("Workspace 2"), std::string::npos);
    EXPECT_NE(summary.find("2 windows"), std::string::npos); // workspace_0 count
}

TEST_F(FilterResultTest, BasicJsonOutput) {
    FilterResult result(testWindows, 10, testQuery, testTime);

    std::string json = result.toJson();

    // Check JSON structure
    EXPECT_EQ(json.front(), '{');
    EXPECT_EQ(json.back(), '}');

    // Check required fields
    EXPECT_NE(json.find("\"windows\""), std::string::npos);
    EXPECT_NE(json.find("\"metadata\""), std::string::npos);
    EXPECT_NE(json.find("\"totalCount\""), std::string::npos);
    EXPECT_NE(json.find("\"filteredCount\""), std::string::npos);
    EXPECT_NE(json.find("\"searchTime\""), std::string::npos);
}

TEST_F(FilterResultTest, EnhancedJsonOutput) {
    FilterResult result(testWindows, 10, testQuery, testTime, testWorkspaces);

    std::string json = result.toJsonWithWorkspaces();

    // Check enhanced JSON structure
    EXPECT_NE(json.find("\"workspaces\""), std::string::npos);
    EXPECT_NE(json.find("\"statistics\""), std::string::npos);
    EXPECT_NE(json.find("\"workspaceCount\""), std::string::npos);

    // Should contain workspace information
    EXPECT_NE(json.find("Workspace 0"), std::string::npos);
    EXPECT_NE(json.find("workspace_0"), std::string::npos);
}

TEST_F(FilterResultTest, CrossWorkspaceStatistics) {
    FilterResult result(testWindows, 10, testQuery, testTime, testWorkspaces);

    auto stats = result.getWorkspaceStatistics();

    EXPECT_EQ(stats.totalWorkspaces, 3);
    EXPECT_EQ(stats.totalWindows, 5);
    EXPECT_EQ(stats.activeWorkspaces, 3); // All workspaces have windows
    EXPECT_EQ(stats.visibleWindows, 3); // windows 0, 2, 4
    EXPECT_EQ(stats.minimizedWindows, 1); // window 2
    EXPECT_EQ(stats.focusedWindows, 1); // window 0
    EXPECT_GT(stats.averageWindowsPerWorkspace, 0);
}

TEST_F(FilterResultTest, StatisticalCounts) {
    FilterResult result(testWindows, 10, testQuery, testTime, testWorkspaces);

    EXPECT_EQ(result.getVisibleWindowCount(), 3);
    EXPECT_EQ(result.getMinimizedWindowCount(), 1);
    EXPECT_EQ(result.getFocusedWindowCount(), 1);
    EXPECT_EQ(result.getActiveWorkspaceCount(), 3);
}

TEST_F(FilterResultTest, WorkspaceDistribution) {
    FilterResult result(testWindows, 10, testQuery, testTime, testWorkspaces);

    auto distribution = result.getWorkspaceDistribution();

    EXPECT_EQ(distribution.size(), 3);
    EXPECT_EQ(distribution["workspace_0"], 2);
    EXPECT_EQ(distribution["workspace_1"], 2);
    EXPECT_EQ(distribution["workspace_2"], 1);
}

TEST_F(FilterResultTest, AverageWindowsPerWorkspace) {
    FilterResult result(testWindows, 10, testQuery, testTime, testWorkspaces);

    double average = result.getAverageWindowsPerWorkspace();
    EXPECT_DOUBLE_EQ(average, 5.0 / 3.0); // 5 windows across 3 workspaces
}

TEST_F(FilterResultTest, CrossWorkspaceStatisticsJson) {
    FilterResult result(testWindows, 10, testQuery, testTime, testWorkspaces);

    std::string statsJson = result.getCrossWorkspaceStatistics();

    // Check statistics fields
    EXPECT_NE(statsJson.find("\"visibleWindows\""), std::string::npos);
    EXPECT_NE(statsJson.find("\"minimizedWindows\""), std::string::npos);
    EXPECT_NE(statsJson.find("\"focusedWindows\""), std::string::npos);
    EXPECT_NE(statsJson.find("\"workspaceDistribution\""), std::string::npos);
    EXPECT_NE(statsJson.find("\"performance\""), std::string::npos);
}

TEST_F(FilterResultTest, SummaryGeneration) {
    FilterResult result(testWindows, 10, testQuery, testTime, testWorkspaces);

    std::string summary = result.getSummary();

    EXPECT_NE(summary.find("5 of 10"), std::string::npos); // filtered count
    EXPECT_NE(summary.find("Window"), std::string::npos); // query
    EXPECT_NE(summary.find("3 workspaces"), std::string::npos); // workspace count
    EXPECT_NE(summary.find("150ms"), std::string::npos); // search time
}

TEST_F(FilterResultTest, ValidationChecks) {
    FilterResult validResult(testWindows, 10, testQuery, testTime);
    EXPECT_TRUE(validResult.isValid());

    // Invalid result - filtered count > total count
    FilterResult invalidResult(testWindows, 3, testQuery, testTime);
    EXPECT_FALSE(invalidResult.isValid());

    // Mismatched counts
    std::vector<WindowInfo> twoWindows = {testWindows[0], testWindows[1]};
    FilterResult mismatchedResult(twoWindows, 10, testQuery, testTime);
    mismatchedResult.filteredCount = 5; // Wrong count
    EXPECT_FALSE(mismatchedResult.isValid());
}

TEST_F(FilterResultTest, EmptyResultHandling) {
    std::vector<WindowInfo> emptyWindows;
    FilterResult emptyResult(emptyWindows, 0, testQuery, testTime, testWorkspaces);

    EXPECT_EQ(emptyResult.filteredCount, 0);
    EXPECT_EQ(emptyResult.getWorkspaceCount(), 0);
    EXPECT_EQ(emptyResult.getVisibleWindowCount(), 0);
    EXPECT_TRUE(emptyResult.isValid());
}

} // namespace Tests
} // namespace WindowManager