#pragma once

#include "../core/window.hpp"
#include "../core/workspace.hpp"
#include "../core/focus_operation.hpp"
#include "../filters/search_query.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <chrono>

namespace WindowManager {

// Forward declarations for filtering support
struct FilterResult;

/**
 * Command-line interface for displaying window information
 * Supports both text and JSON output formats
 */
class CLI {
public:
    CLI() = default;
    ~CLI() = default;

    // Non-copyable, moveable
    CLI(const CLI&) = delete;
    CLI& operator=(const CLI&) = delete;
    CLI(CLI&&) = default;
    CLI& operator=(CLI&&) = default;

    // Output format configuration
    void setOutputFormat(const std::string& format);
    void setVerbose(bool verbose);

    // Display methods for User Story 1
    void displayAllWindows(const std::vector<WindowInfo>& windows);
    void displayWindowInfo(const WindowInfo& window);

    // T040-T041: Enhanced window display with handles
    void displayAllWindowsWithHandles(const std::vector<WindowInfo>& windows, bool handlesOnly = false);
    void displayWindowsHandlesOnly(const std::vector<WindowInfo>& windows);

    // NEW: Enhanced workspace-aware display methods (T029, T030)
    void displayAllWindowsWithWorkspaces(const std::vector<WindowInfo>& windows, const std::vector<WorkspaceInfo>& workspaces);
    void displayWorkspaceGroupedWindows(const std::map<std::string, std::vector<WindowInfo>>& windowsByWorkspace, const std::vector<WorkspaceInfo>& workspaces);

    // Display methods for User Story 2 (Enhanced Search Functionality)
    void displayFilteredResults(const FilterResult& result);
    void displayNoMatches(const std::string& keyword);

    // T037-T039: Enhanced search functionality
    void displayEnhancedSearchResults(const FilterResult& result);
    void displaySearchHelp();
    SearchQuery parseSearchQuery(const std::string& query, const std::vector<std::string>& options = {});

    // T048: Comprehensive help documentation and CLI interface descriptions
    void displayGeneralHelp();
    void displayWorkspaceHelp();
    void displayVersionInfo();
    void displayPerformanceInfo();
    void displayTroubleshootingHelp();

    // T040-T041: Cross-workspace management functionality
    void displayWorkspaceSummary(const std::vector<WorkspaceInfo>& workspaces, const std::vector<WindowInfo>& allWindows);
    void displayWorkspaceStatus(const std::vector<WorkspaceInfo>& workspaces);
    void displayFilteredByWorkspace(const std::vector<WindowInfo>& windows, const std::string& workspaceId, const std::vector<WorkspaceInfo>& workspaces);
    void displayCurrentWorkspaceWindows(const std::vector<WindowInfo>& windows, const std::vector<WorkspaceInfo>& workspaces);

    // T043: Workspace information display helpers
    void displayFocusedWindowInfo(const std::optional<WindowInfo>& focusedWindow, const std::vector<WorkspaceInfo>& workspaces);
    void displayCrossWorkspaceStatistics(const std::vector<WindowInfo>& allWindows, const std::vector<WorkspaceInfo>& workspaces);
    std::string formatWorkspaceInfo(const WorkspaceInfo& workspace, bool includeIndex = true);

    // NEW: Focus command display methods (from contracts/focus_api.md)
    void displayFocusSuccess(const std::string& handle, const std::string& title = "",
                           const std::string& workspace = "", bool workspaceSwitched = false,
                           std::chrono::milliseconds duration = std::chrono::milliseconds(0));
    void displayFocusError(const std::string& handle, const std::string& error,
                         const std::string& suggestion = "");
    void displayFocusProgress(const std::string& handle, const std::string& status);
    void displayHandleValidation(const std::string& handle, bool isValid,
                               const std::string& reason = "");

    // Utility display methods
    void displayError(const std::string& message);
    void displaySuccess(const std::string& message);
    void displayInfo(const std::string& message);
    void displayPerformanceStats(std::chrono::milliseconds duration, size_t windowCount);

    // Input methods (for User Story 3)
    std::string getSearchKeyword();
    bool promptYesNo(const std::string& question);

private:
    std::string outputFormat_ = "text"; // "text" or "json"
    bool verbose_ = false;

    // UI formatting constants
    static constexpr size_t DEFAULT_TITLE_TRUNCATE_LENGTH = 50;
    static constexpr int MILLISECONDS_PER_SECOND = 1000;

    // Text output helpers
    void displayWindowsAsText(const std::vector<WindowInfo>& windows);
    void displayWindowAsText(const WindowInfo& window, int index = -1);
    void displayWindowsWithHandlesAsText(const std::vector<WindowInfo>& windows);

    // NEW: Workspace-aware text output helpers (T030)
    void displayWorkspaceGroupedAsText(const std::map<std::string, std::vector<WindowInfo>>& windowsByWorkspace, const std::vector<WorkspaceInfo>& workspaces);
    void displayWorkspaceHeader(const WorkspaceInfo& workspace);
    void displayWindowWithState(const WindowInfo& window, bool isLastInGroup = false);
    std::string getWindowStateIndicator(const WindowInfo& window);
    std::string formatWorkspaceSummary(const std::vector<WorkspaceInfo>& workspaces, size_t totalWindows);

    // JSON output helpers
    void displayWindowsAsJson(const std::vector<WindowInfo>& windows);
    void displayWorkspaceGroupedAsJson(const std::map<std::string, std::vector<WindowInfo>>& windowsByWorkspace, const std::vector<WorkspaceInfo>& workspaces);
    std::string escapeJsonString(const std::string& str);

    // Formatting helpers
    std::string formatDuration(std::chrono::milliseconds duration);
    std::string formatSize(unsigned int width, unsigned int height);
    std::string formatPosition(int x, int y);
    std::string truncateString(const std::string& str, size_t maxLength);
};

} // namespace WindowManager