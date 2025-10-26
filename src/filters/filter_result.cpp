#include "filter_result.hpp"
#include <sstream>
#include <iomanip>

namespace WindowManager {

FilterResult::FilterResult(std::vector<WindowInfo> windows, size_t total,
                          const SearchQuery& query, std::chrono::milliseconds time)
    : windows(std::move(windows))
    , totalCount(total)
    , filteredCount(this->windows.size())
    , searchTime(time)
    , query(query) {
    // Group windows by workspace automatically
    groupByWorkspace();
}

// T033: Enhanced constructor with workspace information
FilterResult::FilterResult(std::vector<WindowInfo> windows, size_t total,
                          const SearchQuery& query, std::chrono::milliseconds time,
                          const std::vector<WorkspaceInfo>& workspaces)
    : windows(std::move(windows))
    , totalCount(total)
    , filteredCount(this->windows.size())
    , searchTime(time)
    , query(query)
    , workspaces(workspaces) {
    // Group windows by workspace automatically
    groupByWorkspace();
}

double FilterResult::filterRatio() const {
    if (totalCount == 0) {
        return 1.0; // If no windows, ratio is 100%
    }
    return static_cast<double>(filteredCount) / static_cast<double>(totalCount);
}

bool FilterResult::meetsPerformanceTarget() const {
    // Performance requirement: search filtering in <1 second
    return searchTime < std::chrono::milliseconds(1000);
}

std::string FilterResult::getSummary() const {
    std::ostringstream oss;

    if (query.isEmpty()) {
        oss << "All windows (" << filteredCount << " total)";
    } else {
        oss << "Windows (" << filteredCount << " of " << totalCount << ")";
        if (filteredCount == 0) {
            oss << " - No matches found for '" << query.query << "'";
        } else {
            oss << " matching '" << query.query << "'";
        }
    }

    // Add workspace information to summary
    if (!windowsByWorkspace.empty()) {
        oss << " across " << windowsByWorkspace.size() << " workspaces";
    }

    oss << "\nSearch completed in " << searchTime.count() << "ms";

    if (!meetsPerformanceTarget()) {
        oss << " (WARNING: Exceeded 1 second performance target)";
    }

    return oss.str();
}

bool FilterResult::isValid() const {
    // Validate according to data model rules
    if (filteredCount > totalCount) {
        return false;
    }

    if (filteredCount != windows.size()) {
        return false;
    }

    // Validate performance requirement
    if (searchTime > std::chrono::milliseconds(1000)) {
        // Warning, but still valid - just doesn't meet performance target
    }

    return true;
}

// T034: Implement FilterResult grouping and statistics

void FilterResult::groupByWorkspace() {
    windowsByWorkspace.clear();
    windowCountsByWorkspace.clear();

    for (const auto& window : windows) {
        windowsByWorkspace[window.workspaceId].push_back(window);
        windowCountsByWorkspace[window.workspaceId]++;
    }
}

size_t FilterResult::getWorkspaceCount() const {
    return windowsByWorkspace.size();
}

std::vector<std::string> FilterResult::getWorkspaceIds() const {
    std::vector<std::string> ids;
    for (const auto& pair : windowsByWorkspace) {
        ids.push_back(pair.first);
    }
    return ids;
}

size_t FilterResult::getWindowCountForWorkspace(const std::string& workspaceId) const {
    auto it = windowCountsByWorkspace.find(workspaceId);
    return (it != windowCountsByWorkspace.end()) ? it->second : 0;
}

std::string FilterResult::getWorkspaceStatsSummary() const {
    std::ostringstream oss;
    oss << "Workspace Distribution:\n";

    for (const auto& workspace : workspaces) {
        size_t count = getWindowCountForWorkspace(workspace.id);
        if (count > 0) {
            oss << "  " << workspace.name << " (" << workspace.id << "): "
                << count << " windows";
            if (workspace.isCurrent) {
                oss << " [Current]";
            }
            oss << "\n";
        }
    }

    return oss.str();
}

std::string FilterResult::toJson() const {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"windows\": [\n";

    for (size_t i = 0; i < windows.size(); ++i) {
        oss << "    " << windows[i].toJson();
        if (i < windows.size() - 1) {
            oss << ",";
        }
        oss << "\n";
    }

    oss << "  ],\n";
    oss << "  \"metadata\": {\n";
    oss << "    \"totalCount\": " << totalCount << ",\n";
    oss << "    \"filteredCount\": " << filteredCount << ",\n";
    oss << "    \"searchTime\": " << searchTime.count() << ",\n";
    oss << "    \"query\": \"" << query.query << "\",\n";

    // Format timestamp as ISO 8601
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    oss << "    \"timestamp\": \"" << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ") << "\"\n";

    oss << "  }\n";
    oss << "}";

    return oss.str();
}

// T035: Add FilterResult output formatting with workspace info
std::string FilterResult::toJsonWithWorkspaces() const {
    std::ostringstream oss;
    oss << "{\n";

    // Metadata section
    oss << "  \"metadata\": {\n";
    oss << "    \"totalCount\": " << totalCount << ",\n";
    oss << "    \"filteredCount\": " << filteredCount << ",\n";
    oss << "    \"searchTime\": " << searchTime.count() << ",\n";
    oss << "    \"workspaceCount\": " << getWorkspaceCount() << ",\n";
    oss << "    \"query\": {\n";
    oss << "      \"text\": \"" << query.query << "\",\n";
    oss << "      \"field\": \"";

    switch (query.field) {
        case SearchField::Title: oss << "title"; break;
        case SearchField::Owner: oss << "owner"; break;
        case SearchField::Both: oss << "both"; break;
    }

    oss << "\",\n";
    oss << "      \"caseSensitive\": " << (query.caseSensitive ? "true" : "false") << ",\n";
    oss << "      \"useRegex\": " << (query.useRegex ? "true" : "false") << "\n";
    oss << "    },\n";

    // Add cross-workspace statistics
    oss << "    \"statistics\": " << getCrossWorkspaceStatistics() << ",\n";

    // Format timestamp as ISO 8601
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    oss << "    \"timestamp\": \"" << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ") << "\"\n";
    oss << "  },\n";

    // Workspaces section with grouped windows
    oss << "  \"workspaces\": [\n";
    bool firstWorkspace = true;

    for (const auto& workspace : workspaces) {
        auto it = windowsByWorkspace.find(workspace.id);

        if (!firstWorkspace) {
            oss << ",\n";
        }
        firstWorkspace = false;

        const auto& workspaceWindows = (it != windowsByWorkspace.end()) ? it->second : std::vector<WindowInfo>();

        oss << "    {\n";
        oss << "      \"id\": \"" << workspace.id << "\",\n";
        oss << "      \"name\": \"" << workspace.name << "\",\n";
        oss << "      \"index\": " << workspace.index << ",\n";
        oss << "      \"isCurrent\": " << (workspace.isCurrent ? "true" : "false") << ",\n";
        oss << "      \"windowCount\": " << workspaceWindows.size() << ",\n";
        oss << "      \"windows\": [\n";

        for (size_t i = 0; i < workspaceWindows.size(); ++i) {
            oss << "        " << workspaceWindows[i].toJson();
            if (i < workspaceWindows.size() - 1) {
                oss << ",";
            }
            oss << "\n";
        }

        oss << "      ]\n";
        oss << "    }";
    }

    oss << "\n  ]\n";
    oss << "}";

    return oss.str();
}

// T044: Cross-workspace window statistics implementation

std::string FilterResult::getCrossWorkspaceStatistics() const {
    std::ostringstream oss;
    oss << "{\n";

    // Calculate window state statistics across all workspaces
    size_t visibleWindows = 0;
    size_t minimizedWindows = 0;
    size_t focusedWindows = 0;
    size_t hiddenWindows = 0;

    for (const auto& window : windows) {
        if (window.isVisible) visibleWindows++;
        if (window.isMinimized) minimizedWindows++;
        if (window.isFocused) focusedWindows++;
        if (!window.isVisible) hiddenWindows++;
    }

    oss << "      \"visibleWindows\": " << visibleWindows << ",\n";
    oss << "      \"minimizedWindows\": " << minimizedWindows << ",\n";
    oss << "      \"focusedWindows\": " << focusedWindows << ",\n";
    oss << "      \"hiddenWindows\": " << hiddenWindows << ",\n";

    // Add workspace distribution statistics
    oss << "      \"workspaceDistribution\": {\n";
    bool firstWorkspace = true;
    for (const auto& workspace : workspaces) {
        if (!firstWorkspace) {
            oss << ",\n";
        }
        firstWorkspace = false;

        size_t count = getWindowCountForWorkspace(workspace.id);
        oss << "        \"" << workspace.id << "\": " << count;
    }
    oss << "\n      },\n";

    // Performance metrics
    oss << "      \"performance\": {\n";
    oss << "        \"filterRatio\": " << std::fixed << std::setprecision(3) << filterRatio() << ",\n";
    oss << "        \"meetsTarget\": " << (meetsPerformanceTarget() ? "true" : "false") << "\n";
    oss << "      }\n";

    oss << "    }";
    return oss.str();
}

WorkspaceStatistics FilterResult::getWorkspaceStatistics() const {
    WorkspaceStatistics stats;

    // Calculate totals
    stats.totalWorkspaces = workspaces.size();
    stats.totalWindows = windows.size();
    stats.activeWorkspaces = 0;

    // Calculate window state counts
    for (const auto& window : windows) {
        if (window.isVisible) stats.visibleWindows++;
        if (window.isMinimized) stats.minimizedWindows++;
        if (window.isFocused) stats.focusedWindows++;
        if (!window.isVisible) stats.hiddenWindows++;
    }

    // Calculate active workspaces (those with windows)
    for (const auto& workspace : workspaces) {
        if (getWindowCountForWorkspace(workspace.id) > 0) {
            stats.activeWorkspaces++;
        }
    }

    // Build distribution map
    for (const auto& workspace : workspaces) {
        stats.windowsByWorkspace[workspace.id] = getWindowCountForWorkspace(workspace.id);
    }

    // Calculate average windows per workspace
    if (stats.activeWorkspaces > 0) {
        stats.averageWindowsPerWorkspace = static_cast<double>(stats.totalWindows) / stats.activeWorkspaces;
    } else {
        stats.averageWindowsPerWorkspace = 0.0;
    }

    return stats;
}

size_t FilterResult::getVisibleWindowCount() const {
    size_t count = 0;
    for (const auto& window : windows) {
        if (window.isVisible) count++;
    }
    return count;
}

size_t FilterResult::getMinimizedWindowCount() const {
    size_t count = 0;
    for (const auto& window : windows) {
        if (window.isMinimized) count++;
    }
    return count;
}

size_t FilterResult::getFocusedWindowCount() const {
    size_t count = 0;
    for (const auto& window : windows) {
        if (window.isFocused) count++;
    }
    return count;
}

size_t FilterResult::getActiveWorkspaceCount() const {
    size_t count = 0;
    for (const auto& workspace : workspaces) {
        if (getWindowCountForWorkspace(workspace.id) > 0) {
            count++;
        }
    }
    return count;
}

double FilterResult::getAverageWindowsPerWorkspace() const {
    size_t activeWorkspaces = getActiveWorkspaceCount();
    if (activeWorkspaces == 0) {
        return 0.0;
    }
    return static_cast<double>(filteredCount) / activeWorkspaces;
}

std::map<std::string, size_t> FilterResult::getWorkspaceDistribution() const {
    std::map<std::string, size_t> distribution;
    for (const auto& workspace : workspaces) {
        distribution[workspace.id] = getWindowCountForWorkspace(workspace.id);
    }
    return distribution;
}

} // namespace WindowManager