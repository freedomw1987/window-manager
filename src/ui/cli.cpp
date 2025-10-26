#include "cli.hpp"
#include "../filters/filter_result.hpp"
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace WindowManager {

void CLI::setOutputFormat(const std::string& format) {
    if (format == "text" || format == "json") {
        outputFormat_ = format;
    } else {
        throw std::invalid_argument("Invalid output format: " + format + ". Use 'text' or 'json'.");
    }
}

void CLI::setVerbose(bool verbose) {
    verbose_ = verbose;
}

void CLI::displayAllWindows(const std::vector<WindowInfo>& windows) {
    if (outputFormat_ == "json") {
        displayWindowsAsJson(windows);
    } else {
        displayWindowsAsText(windows);
    }
}

void CLI::displayWindowInfo(const WindowInfo& window) {
    if (outputFormat_ == "json") {
        std::cout << window.toJson() << std::endl;
    } else {
        displayWindowAsText(window);
    }
}

// NEW: Enhanced workspace-aware display methods (T029)

void CLI::displayAllWindowsWithWorkspaces(const std::vector<WindowInfo>& windows, const std::vector<WorkspaceInfo>& workspaces) {
    // Group windows by workspace
    std::map<std::string, std::vector<WindowInfo>> windowsByWorkspace;

    for (const auto& window : windows) {
        windowsByWorkspace[window.workspaceId].push_back(window);
    }

    displayWorkspaceGroupedWindows(windowsByWorkspace, workspaces);
}

void CLI::displayWorkspaceGroupedWindows(const std::map<std::string, std::vector<WindowInfo>>& windowsByWorkspace, const std::vector<WorkspaceInfo>& workspaces) {
    if (outputFormat_ == "json") {
        displayWorkspaceGroupedAsJson(windowsByWorkspace, workspaces);
    } else {
        displayWorkspaceGroupedAsText(windowsByWorkspace, workspaces);
    }
}

void CLI::displayError(const std::string& message) {
    if (outputFormat_ == "json") {
        std::cout << "{\n";
        std::cout << "  \"error\": \"" << escapeJsonString(message) << "\"\n";
        std::cout << "}" << std::endl;
    } else {
        std::cerr << "Error: " << message << std::endl;
    }
}

void CLI::displaySuccess(const std::string& message) {
    if (outputFormat_ == "json") {
        std::cout << "{\n";
        std::cout << "  \"status\": \"success\",\n";
        std::cout << "  \"message\": \"" << escapeJsonString(message) << "\"\n";
        std::cout << "}" << std::endl;
    } else {
        std::cout << "✓ " << message << std::endl;
    }
}

void CLI::displayInfo(const std::string& message) {
    if (outputFormat_ == "text") {
        std::cout << message << std::endl;
    }
    // In JSON mode, info messages are typically suppressed unless they're part of the data
}

void CLI::displayFilteredResults(const FilterResult& result) {
    if (outputFormat_ == "json") {
        std::cout << result.toJson() << std::endl;
    } else {
        // Display search summary
        std::cout << result.getSummary() << std::endl;

        if (result.filteredCount > 0) {
            std::cout << std::endl;
            displayWindowsAsText(result.windows);
        }

        // Show performance warning if needed
        if (!result.meetsPerformanceTarget()) {
            std::cout << "\n⚠ Warning: Search took longer than expected ("
                      << result.searchTime.count() << "ms > 1000ms)" << std::endl;
        }

        // Show verbose stats
        if (verbose_) {
            std::cout << "\nSearch Statistics:" << std::endl;
            std::cout << "  Filter ratio: " << std::fixed << std::setprecision(1)
                      << (result.filterRatio() * 100) << "%" << std::endl;
            std::cout << "  Query: " << result.query.toString() << std::endl;
        }
    }
}

void CLI::displayNoMatches(const std::string& keyword) {
    if (outputFormat_ == "json") {
        std::cout << "{\n";
        std::cout << "  \"windows\": [],\n";
        std::cout << "  \"metadata\": {\n";
        std::cout << "    \"totalCount\": 0,\n";
        std::cout << "    \"filteredCount\": 0,\n";
        std::cout << "    \"query\": \"" << escapeJsonString(keyword) << "\",\n";
        std::cout << "    \"message\": \"No windows found matching the search criteria\"\n";
        std::cout << "  }\n";
        std::cout << "}" << std::endl;
    } else {
        std::cout << "No windows found matching '" << keyword << "'" << std::endl;
        std::cout << "\nTips:" << std::endl;
        std::cout << "  • Try a shorter or more general keyword" << std::endl;
        std::cout << "  • Check if the application is running" << std::endl;
        std::cout << "  • Use 'list' command to see all available windows" << std::endl;
    }
}

void CLI::displayPerformanceStats(std::chrono::milliseconds duration, size_t windowCount) {
    if (verbose_) {
        if (outputFormat_ == "json") {
            std::cout << "{\n";
            std::cout << "  \"performance\": {\n";
            std::cout << "    \"enumerationTime\": " << duration.count() << ",\n";
            std::cout << "    \"windowCount\": " << windowCount << "\n";
            std::cout << "  }\n";
            std::cout << "}" << std::endl;
        } else {
            std::cout << "\nPerformance: " << formatDuration(duration)
                      << " to enumerate " << windowCount << " windows" << std::endl;
        }
    }
}

std::string CLI::getSearchKeyword() {
    std::cout << "Search (or 'q' to quit): ";
    std::string keyword;
    std::getline(std::cin, keyword);
    return keyword;
}

bool CLI::promptYesNo(const std::string& question) {
    std::cout << question << " (y/n): ";
    std::string response;
    std::getline(std::cin, response);

    std::transform(response.begin(), response.end(), response.begin(), ::tolower);
    return response == "y" || response == "yes";
}

// Text output implementation
void CLI::displayWindowsAsText(const std::vector<WindowInfo>& windows) {
    if (windows.empty()) {
        std::cout << "No windows found." << std::endl;
        return;
    }

    std::cout << "Windows (" << windows.size() << " total):" << std::endl;

    for (size_t i = 0; i < windows.size(); ++i) {
        displayWindowAsText(windows[i], static_cast<int>(i + 1));

        // Add separator between windows (except for the last one)
        if (i < windows.size() - 1) {
            std::cout << std::endl;
        }
    }
}

void CLI::displayWindowAsText(const WindowInfo& window, int index) {
    std::ostringstream oss;

    if (index > 0) {
        oss << "[" << index << "] ";
    }

    oss << window.ownerName;
    if (!window.title.empty()) {
        oss << " - " << truncateString(window.title, DEFAULT_TITLE_TRUNCATE_LENGTH);
    }

    std::cout << oss.str() << std::endl;

    // Display position and size
    std::cout << "    Position: " << formatPosition(window.x, window.y);
    std::cout << "  Size: " << formatSize(window.width, window.height);
    std::cout << "  PID: " << window.processId;

    if (!window.isVisible) {
        std::cout << "  [Hidden]";
    }

    if (verbose_) {
        std::cout << "  Handle: " << window.handle;
    }

    std::cout << std::endl;
}

// JSON output implementation
void CLI::displayWindowsAsJson(const std::vector<WindowInfo>& windows) {
    std::cout << "{\n";
    std::cout << "  \"windows\": [\n";

    for (size_t i = 0; i < windows.size(); ++i) {
        std::cout << "    " << windows[i].toJson();

        if (i < windows.size() - 1) {
            std::cout << ",";
        }
        std::cout << "\n";
    }

    std::cout << "  ],\n";
    std::cout << "  \"totalCount\": " << windows.size() << ",\n";

    // Add timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto* tm = std::gmtime(&time_t);

    std::cout << "  \"timestamp\": \"";
    std::cout << std::put_time(tm, "%Y-%m-%dT%H:%M:%SZ");
    std::cout << "\"\n";
    std::cout << "}" << std::endl;
}

// Helper methods
std::string CLI::escapeJsonString(const std::string& str) {
    std::ostringstream oss;
    for (char c : str) {
        switch (c) {
            case '"': oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:
                if (c < 0x20) {
                    oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
                } else {
                    oss << c;
                }
                break;
        }
    }
    return oss.str();
}

std::string CLI::formatDuration(std::chrono::milliseconds duration) {
    if (duration.count() < MILLISECONDS_PER_SECOND) {
        return std::to_string(duration.count()) + "ms";
    } else {
        double seconds = duration.count() / static_cast<double>(MILLISECONDS_PER_SECOND);
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << seconds << "s";
        return oss.str();
    }
}

std::string CLI::formatSize(unsigned int width, unsigned int height) {
    return std::to_string(width) + "x" + std::to_string(height);
}

std::string CLI::formatPosition(int x, int y) {
    return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
}

std::string CLI::truncateString(const std::string& str, size_t maxLength) {
    if (str.length() <= maxLength) {
        return str;
    }
    return str.substr(0, maxLength - 3) + "...";
}

// NEW: Workspace-aware display implementations (T030)

void CLI::displayWorkspaceGroupedAsText(const std::map<std::string, std::vector<WindowInfo>>& windowsByWorkspace, const std::vector<WorkspaceInfo>& workspaces) {
    if (windowsByWorkspace.empty()) {
        std::cout << "No windows found." << std::endl;
        return;
    }

    size_t totalWindows = 0;
    for (const auto& pair : windowsByWorkspace) {
        totalWindows += pair.second.size();
    }

    // Display windows grouped by workspace, ordered by workspace index
    bool firstWorkspace = true;
    for (const auto& workspace : workspaces) {
        auto it = windowsByWorkspace.find(workspace.id);
        if (it == windowsByWorkspace.end() || it->second.empty()) {
            continue; // Skip workspaces with no windows
        }

        if (!firstWorkspace) {
            std::cout << std::endl;
        }
        firstWorkspace = false;

        displayWorkspaceHeader(workspace);

        const auto& windowsInWorkspace = it->second;
        for (size_t i = 0; i < windowsInWorkspace.size(); ++i) {
            bool isLast = (i == windowsInWorkspace.size() - 1);
            displayWindowWithState(windowsInWorkspace[i], isLast);
        }
    }

    // Display summary
    std::cout << std::endl;
    std::cout << formatWorkspaceSummary(workspaces, totalWindows) << std::endl;
}

void CLI::displayWorkspaceHeader(const WorkspaceInfo& workspace) {
    std::cout << "Workspace: " << workspace.name
              << " (ID: " << workspace.id;

    if (workspace.isCurrent) {
        std::cout << ", Current: Yes";
    } else {
        std::cout << ", Current: No";
    }

    std::cout << ")" << std::endl;
}

void CLI::displayWindowWithState(const WindowInfo& window, bool isLastInGroup) {
    std::ostringstream oss;

    // Add tree-like connector
    if (isLastInGroup) {
        oss << "└── ";
    } else {
        oss << "├── ";
    }

    // Add window title and owner
    if (!window.title.empty()) {
        oss << truncateString(window.title, DEFAULT_TITLE_TRUNCATE_LENGTH - 20);
    } else {
        oss << window.ownerName;
    }

    // Add spacing to align state indicators
    std::string content = oss.str();
    if (content.length() < 40) {
        content += std::string(40 - content.length(), ' ');
    }

    std::cout << content;

    // Add process ID and state indicator
    std::cout << " [PID: " << window.processId << "] ";
    std::cout << getWindowStateIndicator(window) << std::endl;
}

std::string CLI::getWindowStateIndicator(const WindowInfo& window) {
    switch (window.state) {
        case WindowState::Focused:
            return "[State: Focused]";
        case WindowState::Minimized:
            return "[State: Minimized]";
        case WindowState::Hidden:
            return "[State: Hidden]";
        case WindowState::Normal:
        default:
            return "[State: Normal]";
    }
}

std::string CLI::formatWorkspaceSummary(const std::vector<WorkspaceInfo>& workspaces, size_t totalWindows) {
    std::ostringstream oss;
    oss << "Total: " << totalWindows << " windows across " << workspaces.size() << " workspaces";
    return oss.str();
}

void CLI::displayWorkspaceGroupedAsJson(const std::map<std::string, std::vector<WindowInfo>>& windowsByWorkspace, const std::vector<WorkspaceInfo>& workspaces) {
    std::cout << "{\n";

    // Metadata section
    size_t totalWindows = 0;
    for (const auto& pair : windowsByWorkspace) {
        totalWindows += pair.second.size();
    }

    std::string currentWorkspaceId;
    for (const auto& workspace : workspaces) {
        if (workspace.isCurrent) {
            currentWorkspaceId = workspace.id;
            break;
        }
    }

    std::cout << "  \"metadata\": {\n";
    std::cout << "    \"totalWindows\": " << totalWindows << ",\n";
    std::cout << "    \"totalWorkspaces\": " << workspaces.size() << ",\n";
    std::cout << "    \"currentWorkspace\": \"" << escapeJsonString(currentWorkspaceId) << "\",\n";
    std::cout << "    \"platform\": \"Cross-platform window manager\"\n";
    std::cout << "  },\n";

    // Workspaces section
    std::cout << "  \"workspaces\": [\n";

    bool firstWorkspace = true;
    for (const auto& workspace : workspaces) {
        auto it = windowsByWorkspace.find(workspace.id);
        if (it == windowsByWorkspace.end()) {
            // Include workspace even if it has no windows
        }

        if (!firstWorkspace) {
            std::cout << ",\n";
        }
        firstWorkspace = false;

        const auto& windowsInWorkspace = (it != windowsByWorkspace.end()) ? it->second : std::vector<WindowInfo>();

        std::cout << "    {\n";
        std::cout << "      \"id\": \"" << escapeJsonString(workspace.id) << "\",\n";
        std::cout << "      \"name\": \"" << escapeJsonString(workspace.name) << "\",\n";
        std::cout << "      \"index\": " << workspace.index << ",\n";
        std::cout << "      \"isCurrent\": " << (workspace.isCurrent ? "true" : "false") << ",\n";
        std::cout << "      \"windowCount\": " << windowsInWorkspace.size() << ",\n";
        std::cout << "      \"windows\": [\n";

        for (size_t i = 0; i < windowsInWorkspace.size(); ++i) {
            std::cout << "        " << windowsInWorkspace[i].toJson();
            if (i < windowsInWorkspace.size() - 1) {
                std::cout << ",";
            }
            std::cout << "\n";
        }

        std::cout << "      ]\n";
        std::cout << "    }";
    }

    std::cout << "\n  ]\n";
    std::cout << "}" << std::endl;
}

// T037-T039: Enhanced search functionality implementation

void CLI::displayEnhancedSearchResults(const FilterResult& result) {
    if (outputFormat_ == "json") {
        // Use enhanced JSON output with workspace grouping
        std::cout << result.toJsonWithWorkspaces() << std::endl;
    } else {
        // Enhanced text display with workspace grouping
        std::cout << result.getSummary() << std::endl;

        if (result.filteredCount > 0) {
            std::cout << std::endl;

            // Display workspace statistics if available
            if (!result.workspaces.empty()) {
                std::cout << result.getWorkspaceStatsSummary() << std::endl;
            }

            // Display grouped windows by workspace
            displayWorkspaceGroupedWindows(result.windowsByWorkspace, result.workspaces);
        }

        // Show performance warning if needed
        if (!result.meetsPerformanceTarget()) {
            std::cout << "\n⚠ Warning: Search took longer than expected ("
                      << result.searchTime.count() << "ms > 1000ms)" << std::endl;
        }

        // Show verbose stats
        if (verbose_) {
            std::cout << "\nSearch Statistics:" << std::endl;
            std::cout << "  Filter ratio: " << std::fixed << std::setprecision(1)
                      << (result.filterRatio() * 100) << "%" << std::endl;
            std::cout << "  Query: " << result.query.toString() << std::endl;
            std::cout << "  Workspaces found: " << result.getWorkspaceCount() << std::endl;
        }
    }
}

void CLI::displaySearchHelp() {
    if (outputFormat_ == "text") {
        std::cout << "Enhanced Search Help:" << std::endl;
        std::cout << "  Search both title and application name by default" << std::endl;
        std::cout << "  Examples:" << std::endl;
        std::cout << "    search chrome                    # Find all Chrome windows" << std::endl;
        std::cout << "    search --field=title \"My Doc\"    # Search only window titles" << std::endl;
        std::cout << "    search --field=owner code       # Search only application names" << std::endl;
        std::cout << "    search --case-sensitive Git      # Case-sensitive search" << std::endl;
        std::cout << "    search --regex \"^Visual.*\"       # Regular expression search" << std::endl;
        std::cout << "    search --workspace=0 terminal    # Search within specific workspace" << std::endl;
        std::cout << std::endl;
        std::cout << "Search Fields:" << std::endl;
        std::cout << "  title    - Search window titles only" << std::endl;
        std::cout << "  owner    - Search application names only" << std::endl;
        std::cout << "  both     - Search both titles and app names (default)" << std::endl;
    }
}

// T048: Comprehensive help documentation and CLI interface descriptions

void CLI::displayGeneralHelp() {
    if (outputFormat_ == "json") {
        std::cout << "{" << std::endl;
        std::cout << "  \"help\": {" << std::endl;
        std::cout << "    \"commands\": [" << std::endl;
        std::cout << "      {\"name\": \"list\", \"description\": \"List all windows with workspace information\"}," << std::endl;
        std::cout << "      {\"name\": \"search\", \"description\": \"Search windows by title or application name\"}," << std::endl;
        std::cout << "      {\"name\": \"workspaces\", \"description\": \"Show workspace summary and status\"}," << std::endl;
        std::cout << "      {\"name\": \"focus\", \"description\": \"Show currently focused window information\"}," << std::endl;
        std::cout << "      {\"name\": \"stats\", \"description\": \"Display cross-workspace statistics\"}" << std::endl;
        std::cout << "    ]," << std::endl;
        std::cout << "    \"options\": [" << std::endl;
        std::cout << "      {\"name\": \"--format\", \"values\": [\"text\", \"json\"], \"description\": \"Output format\"}," << std::endl;
        std::cout << "      {\"name\": \"--verbose\", \"description\": \"Enable verbose output with additional details\"}" << std::endl;
        std::cout << "    ]" << std::endl;
        std::cout << "  }" << std::endl;
        std::cout << "}" << std::endl;
        return;
    }

    std::cout << "Cross-Platform Window Manager - Enhanced Workspace Edition" << std::endl;
    std::cout << "==========================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "OVERVIEW:" << std::endl;
    std::cout << "  A powerful cross-platform window management tool that provides" << std::endl;
    std::cout << "  comprehensive window listing, search, and workspace management" << std::endl;
    std::cout << "  capabilities across Windows, macOS, and Linux systems." << std::endl;
    std::cout << std::endl;
    std::cout << "MAIN COMMANDS:" << std::endl;
    std::cout << "  list              List all windows with workspace information" << std::endl;
    std::cout << "  search <query>    Search windows by title or application name" << std::endl;
    std::cout << "  workspaces        Show workspace summary and current status" << std::endl;
    std::cout << "  workspace <id>    Show windows in specific workspace" << std::endl;
    std::cout << "  current           Show windows in current workspace only" << std::endl;
    std::cout << "  focus             Show currently focused window information" << std::endl;
    std::cout << "  stats             Display comprehensive cross-workspace statistics" << std::endl;
    std::cout << "  help              Show this help information" << std::endl;
    std::cout << "  help search       Show detailed search help" << std::endl;
    std::cout << "  help workspaces   Show workspace management help" << std::endl;
    std::cout << std::endl;
    std::cout << "GLOBAL OPTIONS:" << std::endl;
    std::cout << "  --format=<text|json>  Output format (default: text)" << std::endl;
    std::cout << "  --verbose             Enable verbose output with additional details" << std::endl;
    std::cout << std::endl;
    std::cout << "EXAMPLES:" << std::endl;
    std::cout << "  " << "list --verbose                    # List all windows with detailed info" << std::endl;
    std::cout << "  " << "search chrome                     # Find all Chrome windows" << std::endl;
    std::cout << "  " << "search --field=title \"Document\"   # Search only window titles" << std::endl;
    std::cout << "  " << "workspaces --format=json          # Get workspace info as JSON" << std::endl;
    std::cout << "  " << "workspace 1                       # Show windows in workspace 1" << std::endl;
    std::cout << "  " << "current                           # Show current workspace windows" << std::endl;
    std::cout << "  " << "focus --verbose                   # Detailed focused window info" << std::endl;
    std::cout << std::endl;
    std::cout << "SUPPORTED PLATFORMS:" << std::endl;
    std::cout << "  Windows     Virtual Desktops (Windows 10+)" << std::endl;
    std::cout << "  macOS       Spaces and Mission Control" << std::endl;
    std::cout << "  Linux       EWMH-compatible window managers" << std::endl;
    std::cout << std::endl;
    std::cout << "For detailed help on specific features, use 'help <command>'" << std::endl;
}

void CLI::displayWorkspaceHelp() {
    if (outputFormat_ == "json") {
        std::cout << "{" << std::endl;
        std::cout << "  \"workspaceHelp\": {" << std::endl;
        std::cout << "    \"description\": \"Workspace management commands for cross-desktop window operations\"," << std::endl;
        std::cout << "    \"commands\": [" << std::endl;
        std::cout << "      {\"name\": \"workspaces\", \"description\": \"List all workspaces with window counts\"}," << std::endl;
        std::cout << "      {\"name\": \"workspace <id>\", \"description\": \"Show windows in specific workspace\"}," << std::endl;
        std::cout << "      {\"name\": \"current\", \"description\": \"Show windows in current workspace only\"}," << std::endl;
        std::cout << "      {\"name\": \"focus\", \"description\": \"Show focused window with workspace context\"}" << std::endl;
        std::cout << "    ]" << std::endl;
        std::cout << "  }" << std::endl;
        std::cout << "}" << std::endl;
        return;
    }

    std::cout << "Workspace Management Help:" << std::endl;
    std::cout << "==========================" << std::endl;
    std::cout << std::endl;
    std::cout << "WORKSPACE COMMANDS:" << std::endl;
    std::cout << "  workspaces            Show all workspaces with window counts and status" << std::endl;
    std::cout << "  workspace <id>        Show all windows in a specific workspace" << std::endl;
    std::cout << "  current               Show windows in the current workspace only" << std::endl;
    std::cout << "  focus                 Show currently focused window with workspace context" << std::endl;
    std::cout << "  stats                 Show comprehensive cross-workspace statistics" << std::endl;
    std::cout << std::endl;
    std::cout << "WORKSPACE FEATURES:" << std::endl;
    std::cout << "  • Cross-workspace window visibility" << std::endl;
    std::cout << "  • Window state tracking (focused, minimized, hidden)" << std::endl;
    std::cout << "  • Workspace-specific filtering and search" << std::endl;
    std::cout << "  • Current workspace detection" << std::endl;
    std::cout << "  • Multi-desktop window management" << std::endl;
    std::cout << std::endl;
    std::cout << "EXAMPLES:" << std::endl;
    std::cout << "  workspaces                        # Show all workspaces" << std::endl;
    std::cout << "  workspace 0                       # Show windows in workspace 0" << std::endl;
    std::cout << "  workspace \"Development\"            # Show windows in named workspace" << std::endl;
    std::cout << "  current --verbose                 # Detailed current workspace info" << std::endl;
    std::cout << "  focus --format=json               # Focused window info as JSON" << std::endl;
    std::cout << "  search --workspace=1 \"browser\"    # Search within specific workspace" << std::endl;
    std::cout << std::endl;
    std::cout << "PLATFORM SUPPORT:" << std::endl;
    std::cout << "  Windows    Uses Virtual Desktop Manager API" << std::endl;
    std::cout << "  macOS      Uses Core Graphics and Accessibility APIs" << std::endl;
    std::cout << "  Linux      Uses EWMH (_NET_WM_DESKTOP and related properties)" << std::endl;
    std::cout << std::endl;
    std::cout << "NOTE: If workspace support is not available on your system," << std::endl;
    std::cout << "      the tool will gracefully fallback to basic window enumeration." << std::endl;
}

void CLI::displayVersionInfo() {
    if (outputFormat_ == "json") {
        std::cout << "{" << std::endl;
        std::cout << "  \"version\": {" << std::endl;
        std::cout << "    \"application\": \"Cross-Platform Window Manager\"," << std::endl;
        std::cout << "    \"version\": \"2.0.0\"," << std::endl;
        std::cout << "    \"build\": \"Enhanced Workspace Edition\"," << std::endl;
        std::cout << "    \"features\": [" << std::endl;
        std::cout << "      \"Enhanced Window Listing\"," << std::endl;
        std::cout << "      \"Extended Search Functionality\"," << std::endl;
        std::cout << "      \"Cross-Workspace Window Management\"" << std::endl;
        std::cout << "    ]," << std::endl;
        std::cout << "    \"platforms\": [\"Windows\", \"macOS\", \"Linux\"]" << std::endl;
        std::cout << "  }" << std::endl;
        std::cout << "}" << std::endl;
        return;
    }

    std::cout << "Cross-Platform Window Manager v2.0.0" << std::endl;
    std::cout << "Enhanced Workspace Edition" << std::endl;
    std::cout << std::endl;
    std::cout << "FEATURES:" << std::endl;
    std::cout << "  ✓ Enhanced Window Listing with workspace information" << std::endl;
    std::cout << "  ✓ Extended Search Functionality (title + application name)" << std::endl;
    std::cout << "  ✓ Cross-Workspace Window Management" << std::endl;
    std::cout << "  ✓ Real-time window state tracking" << std::endl;
    std::cout << "  ✓ Performance optimization with caching" << std::endl;
    std::cout << "  ✓ Comprehensive error handling" << std::endl;
    std::cout << "  ✓ JSON and text output formats" << std::endl;
    std::cout << std::endl;
    std::cout << "SUPPORTED PLATFORMS:" << std::endl;
    std::cout << "  Windows 10+ (Virtual Desktops)" << std::endl;
    std::cout << "  macOS (Spaces)" << std::endl;
    std::cout << "  Linux (EWMH-compatible WMs)" << std::endl;
    std::cout << std::endl;
    std::cout << "BUILD INFO:" << std::endl;
    std::cout << "  C++17 with CMake 3.16+" << std::endl;
    std::cout << "  Cross-platform compatibility" << std::endl;
    std::cout << "  Performance targets: <3s enumeration, <1s filtering" << std::endl;
}

void CLI::displayPerformanceInfo() {
    if (outputFormat_ == "json") {
        std::cout << "{" << std::endl;
        std::cout << "  \"performance\": {" << std::endl;
        std::cout << "    \"targets\": {" << std::endl;
        std::cout << "      \"windowEnumeration\": \"< 3 seconds\"," << std::endl;
        std::cout << "      \"searchFiltering\": \"< 1 second\"," << std::endl;
        std::cout << "      \"workspaceEnumeration\": \"< 1 second\"" << std::endl;
        std::cout << "    }," << std::endl;
        std::cout << "    \"optimization\": {" << std::endl;
        std::cout << "      \"windowCaching\": \"5 second TTL\"," << std::endl;
        std::cout << "      \"workspaceCaching\": \"10 second TTL\"," << std::endl;
        std::cout << "      \"maxCacheSize\": \"10,000 windows\"" << std::endl;
        std::cout << "    }" << std::endl;
        std::cout << "  }" << std::endl;
        std::cout << "}" << std::endl;
        return;
    }

    std::cout << "Performance Information:" << std::endl;
    std::cout << "=======================" << std::endl;
    std::cout << std::endl;
    std::cout << "PERFORMANCE TARGETS:" << std::endl;
    std::cout << "  Window Enumeration    < 3 seconds (even with 50+ windows)" << std::endl;
    std::cout << "  Search Filtering      < 1 second" << std::endl;
    std::cout << "  Workspace Enumeration < 1 second" << std::endl;
    std::cout << std::endl;
    std::cout << "OPTIMIZATION FEATURES:" << std::endl;
    std::cout << "  Window Caching        5 second TTL (configurable)" << std::endl;
    std::cout << "  Workspace Caching     10 second TTL (workspaces change less)" << std::endl;
    std::cout << "  Memory Management     Max 10,000 windows in cache" << std::endl;
    std::cout << "  Graceful Degradation  Fallback to basic enumeration if needed" << std::endl;
    std::cout << std::endl;
    std::cout << "CACHE BEHAVIOR:" << std::endl;
    std::cout << "  • Automatic invalidation on data staleness" << std::endl;
    std::cout << "  • Priority given to visible windows when cache is full" << std::endl;
    std::cout << "  • Sorted results for consistent output" << std::endl;
    std::cout << "  • Thread-safe operation" << std::endl;
    std::cout << std::endl;
    std::cout << "TROUBLESHOOTING:" << std::endl;
    std::cout << "  • Use --verbose to see performance metrics" << std::endl;
    std::cout << "  • Enable caching if disabled for better performance" << std::endl;
    std::cout << "  • Consider filtering results for large window counts" << std::endl;
}

void CLI::displayTroubleshootingHelp() {
    if (outputFormat_ == "json") {
        std::cout << "{" << std::endl;
        std::cout << "  \"troubleshooting\": {" << std::endl;
        std::cout << "    \"common_issues\": [" << std::endl;
        std::cout << "      {\"issue\": \"Permission denied\", \"solution\": \"Grant accessibility permissions\"}," << std::endl;
        std::cout << "      {\"issue\": \"No workspace support\", \"solution\": \"Use EWMH-compatible window manager\"}," << std::endl;
        std::cout << "      {\"issue\": \"Slow performance\", \"solution\": \"Enable caching and use filters\"}" << std::endl;
        std::cout << "    ]" << std::endl;
        std::cout << "  }" << std::endl;
        std::cout << "}" << std::endl;
        return;
    }

    std::cout << "Troubleshooting Guide:" << std::endl;
    std::cout << "=====================" << std::endl;
    std::cout << std::endl;
    std::cout << "COMMON ISSUES AND SOLUTIONS:" << std::endl;
    std::cout << std::endl;
    std::cout << "1. Permission Denied Errors:" << std::endl;
    std::cout << "   macOS:   Grant Accessibility permissions in System Preferences" << std::endl;
    std::cout << "            → Security & Privacy → Privacy → Accessibility" << std::endl;
    std::cout << "   Windows: Run as Administrator for enhanced window information" << std::endl;
    std::cout << "   Linux:   Ensure your user has access to X11 display" << std::endl;
    std::cout << std::endl;
    std::cout << "2. No Workspace Support:" << std::endl;
    std::cout << "   Windows: Requires Windows 10 version 1803 or later" << std::endl;
    std::cout << "   macOS:   Supported on all modern versions" << std::endl;
    std::cout << "   Linux:   Use EWMH-compatible window manager (GNOME, KDE, etc.)" << std::endl;
    std::cout << std::endl;
    std::cout << "3. Slow Performance:" << std::endl;
    std::cout << "   • Enable caching (usually enabled by default)" << std::endl;
    std::cout << "   • Use search filters to reduce result set" << std::endl;
    std::cout << "   • Check system resources and close unnecessary windows" << std::endl;
    std::cout << "   • Use --verbose to see performance metrics" << std::endl;
    std::cout << std::endl;
    std::cout << "4. Empty or Missing Results:" << std::endl;
    std::cout << "   • Check if windows are on different workspaces" << std::endl;
    std::cout << "   • Use 'list' command to see all available windows" << std::endl;
    std::cout << "   • Try search with different keywords or fields" << std::endl;
    std::cout << "   • Verify applications are actually running" << std::endl;
    std::cout << std::endl;
    std::cout << "5. JSON Output Issues:" << std::endl;
    std::cout << "   • Ensure --format=json is specified" << std::endl;
    std::cout << "   • Check for special characters in window titles" << std::endl;
    std::cout << "   • Use text format for debugging" << std::endl;
    std::cout << std::endl;
    std::cout << "DIAGNOSTIC COMMANDS:" << std::endl;
    std::cout << "  list --verbose                 # See detailed enumeration info" << std::endl;
    std::cout << "  workspaces                     # Check workspace support" << std::endl;
    std::cout << "  stats                          # System performance overview" << std::endl;
    std::cout << "  help performance               # Performance targets and optimization" << std::endl;
    std::cout << std::endl;
    std::cout << "For additional support, please report issues with:" << std::endl;
    std::cout << "• Your operating system and version" << std::endl;
    std::cout << "• Output of 'list --verbose'" << std::endl;
    std::cout << "• Any error messages encountered" << std::endl;
}

SearchQuery CLI::parseSearchQuery(const std::string& query, const std::vector<std::string>& options) {
    // T038: Add backward-compatible search parameter parsing

    SearchQuery searchQuery(query); // Default constructor with query

    // Parse options
    for (const auto& option : options) {
        if (option == "--case-sensitive") {
            searchQuery.caseSensitive = true;
        } else if (option == "--regex") {
            searchQuery.useRegex = true;
        } else if (option.find("--field=") == 0) {
            std::string fieldValue = option.substr(8); // Remove "--field="
            if (fieldValue == "title") {
                searchQuery.field = SearchField::Title;
            } else if (fieldValue == "owner") {
                searchQuery.field = SearchField::Owner;
            } else if (fieldValue == "both") {
                searchQuery.field = SearchField::Both;
            }
        } else if (option.find("--workspace=") == 0) {
            searchQuery.workspaceFilter = option.substr(12); // Remove "--workspace="
        }
    }

    return searchQuery;
}

// T040-T041: Cross-workspace management functionality implementation

void CLI::displayWorkspaceSummary(const std::vector<WorkspaceInfo>& workspaces, const std::vector<WindowInfo>& allWindows) {
    if (outputFormat_ == "json") {
        std::cout << "{\n";
        std::cout << "  \"workspaces\": [\n";

        bool firstWorkspace = true;
        for (const auto& workspace : workspaces) {
            if (!firstWorkspace) {
                std::cout << ",\n";
            }
            firstWorkspace = false;

            // Count windows in this workspace
            size_t windowCount = 0;
            size_t visibleCount = 0;
            size_t minimizedCount = 0;
            size_t focusedCount = 0;

            for (const auto& window : allWindows) {
                if (window.workspaceId == workspace.id) {
                    windowCount++;
                    if (window.isVisible) visibleCount++;
                    if (window.isMinimized) minimizedCount++;
                    if (window.isFocused) focusedCount++;
                }
            }

            std::cout << "    {\n";
            std::cout << "      \"id\": \"" << escapeJsonString(workspace.id) << "\",\n";
            std::cout << "      \"name\": \"" << escapeJsonString(workspace.name) << "\",\n";
            std::cout << "      \"index\": " << workspace.index << ",\n";
            std::cout << "      \"isCurrent\": " << (workspace.isCurrent ? "true" : "false") << ",\n";
            std::cout << "      \"windowCount\": " << windowCount << ",\n";
            std::cout << "      \"visibleCount\": " << visibleCount << ",\n";
            std::cout << "      \"minimizedCount\": " << minimizedCount << ",\n";
            std::cout << "      \"focusedCount\": " << focusedCount << "\n";
            std::cout << "    }";
        }

        std::cout << "\n  ],\n";
        std::cout << "  \"totalWorkspaces\": " << workspaces.size() << ",\n";
        std::cout << "  \"totalWindows\": " << allWindows.size() << "\n";
        std::cout << "}" << std::endl;
    } else {
        std::cout << "Workspace Summary:" << std::endl;
        std::cout << "==================" << std::endl;

        for (const auto& workspace : workspaces) {
            // Count windows in this workspace
            size_t windowCount = 0;
            size_t visibleCount = 0;
            size_t minimizedCount = 0;
            size_t focusedCount = 0;

            for (const auto& window : allWindows) {
                if (window.workspaceId == workspace.id) {
                    windowCount++;
                    if (window.isVisible) visibleCount++;
                    if (window.isMinimized) minimizedCount++;
                    if (window.isFocused) focusedCount++;
                }
            }

            std::cout << workspace.name << " (ID: " << workspace.id << ")";
            if (workspace.isCurrent) {
                std::cout << " [CURRENT]";
            }
            std::cout << std::endl;

            std::cout << "  Windows: " << windowCount;
            if (windowCount > 0) {
                std::cout << " (Visible: " << visibleCount;
                if (minimizedCount > 0) {
                    std::cout << ", Minimized: " << minimizedCount;
                }
                if (focusedCount > 0) {
                    std::cout << ", Focused: " << focusedCount;
                }
                std::cout << ")";
            }
            std::cout << std::endl;
            std::cout << std::endl;
        }

        std::cout << "Total: " << workspaces.size() << " workspaces, " << allWindows.size() << " windows" << std::endl;
    }
}

void CLI::displayWorkspaceStatus(const std::vector<WorkspaceInfo>& workspaces) {
    if (outputFormat_ == "json") {
        std::cout << "{\n";
        std::cout << "  \"currentWorkspace\": {\n";

        WorkspaceInfo currentWorkspace;
        bool found = false;
        for (const auto& workspace : workspaces) {
            if (workspace.isCurrent) {
                currentWorkspace = workspace;
                found = true;
                break;
            }
        }

        if (found) {
            std::cout << "    \"id\": \"" << escapeJsonString(currentWorkspace.id) << "\",\n";
            std::cout << "    \"name\": \"" << escapeJsonString(currentWorkspace.name) << "\",\n";
            std::cout << "    \"index\": " << currentWorkspace.index << "\n";
        } else {
            std::cout << "    \"id\": null,\n";
            std::cout << "    \"name\": null,\n";
            std::cout << "    \"index\": -1\n";
        }

        std::cout << "  },\n";
        std::cout << "  \"availableWorkspaces\": " << workspaces.size() << "\n";
        std::cout << "}" << std::endl;
    } else {
        std::cout << "Workspace Status:" << std::endl;
        std::cout << "=================" << std::endl;

        for (const auto& workspace : workspaces) {
            if (workspace.isCurrent) {
                std::cout << "Current: " << workspace.name << " (ID: " << workspace.id << ", Index: " << workspace.index << ")" << std::endl;
                break;
            }
        }

        std::cout << "Available workspaces: " << workspaces.size() << std::endl;

        if (verbose_) {
            std::cout << "\nAll workspaces:" << std::endl;
            for (const auto& workspace : workspaces) {
                std::cout << "  [" << workspace.index << "] " << workspace.name << " (ID: " << workspace.id << ")";
                if (workspace.isCurrent) {
                    std::cout << " [CURRENT]";
                }
                std::cout << std::endl;
            }
        }
    }
}

void CLI::displayFilteredByWorkspace(const std::vector<WindowInfo>& windows, const std::string& workspaceId, const std::vector<WorkspaceInfo>& workspaces) {
    // Find workspace name
    std::string workspaceName = workspaceId;
    for (const auto& workspace : workspaces) {
        if (workspace.id == workspaceId) {
            workspaceName = workspace.name;
            break;
        }
    }

    // Filter windows for the specified workspace
    std::vector<WindowInfo> filteredWindows;
    for (const auto& window : windows) {
        if (window.workspaceId == workspaceId) {
            filteredWindows.push_back(window);
        }
    }

    if (outputFormat_ == "json") {
        std::cout << "{\n";
        std::cout << "  \"workspace\": {\n";
        std::cout << "    \"id\": \"" << escapeJsonString(workspaceId) << "\",\n";
        std::cout << "    \"name\": \"" << escapeJsonString(workspaceName) << "\"\n";
        std::cout << "  },\n";
        std::cout << "  \"windows\": [\n";

        for (size_t i = 0; i < filteredWindows.size(); ++i) {
            std::cout << "    " << filteredWindows[i].toJson();
            if (i < filteredWindows.size() - 1) {
                std::cout << ",";
            }
            std::cout << "\n";
        }

        std::cout << "  ],\n";
        std::cout << "  \"windowCount\": " << filteredWindows.size() << "\n";
        std::cout << "}" << std::endl;
    } else {
        std::cout << "Windows in workspace: " << workspaceName << " (ID: " << workspaceId << ")" << std::endl;
        std::cout << "===================================================" << std::endl;

        if (filteredWindows.empty()) {
            std::cout << "No windows found in this workspace." << std::endl;
        } else {
            for (size_t i = 0; i < filteredWindows.size(); ++i) {
                displayWindowWithState(filteredWindows[i], i == filteredWindows.size() - 1);
            }
            std::cout << std::endl;
            std::cout << "Total: " << filteredWindows.size() << " windows" << std::endl;
        }
    }
}

void CLI::displayCurrentWorkspaceWindows(const std::vector<WindowInfo>& windows, const std::vector<WorkspaceInfo>& workspaces) {
    // Find current workspace
    std::string currentWorkspaceId;
    std::string currentWorkspaceName;
    for (const auto& workspace : workspaces) {
        if (workspace.isCurrent) {
            currentWorkspaceId = workspace.id;
            currentWorkspaceName = workspace.name;
            break;
        }
    }

    displayFilteredByWorkspace(windows, currentWorkspaceId, workspaces);
}

// T043: Workspace information display helpers

void CLI::displayFocusedWindowInfo(const std::optional<WindowInfo>& focusedWindow, const std::vector<WorkspaceInfo>& workspaces) {
    if (outputFormat_ == "json") {
        std::cout << "{\n";
        std::cout << "  \"focusedWindow\": ";

        if (focusedWindow) {
            std::cout << focusedWindow->toJson();
        } else {
            std::cout << "null";
        }

        std::cout << ",\n";
        std::cout << "  \"workspace\": ";

        if (focusedWindow) {
            // Find the workspace for the focused window
            for (const auto& workspace : workspaces) {
                if (workspace.id == focusedWindow->workspaceId) {
                    std::cout << "{\n";
                    std::cout << "    \"id\": \"" << escapeJsonString(workspace.id) << "\",\n";
                    std::cout << "    \"name\": \"" << escapeJsonString(workspace.name) << "\",\n";
                    std::cout << "    \"isCurrent\": " << (workspace.isCurrent ? "true" : "false") << "\n";
                    std::cout << "  }";
                    break;
                }
            }
        } else {
            std::cout << "null";
        }

        std::cout << "\n}" << std::endl;
    } else {
        std::cout << "Focused Window Information:" << std::endl;
        std::cout << "==========================" << std::endl;

        if (focusedWindow) {
            std::cout << "Title: " << focusedWindow->title << std::endl;
            std::cout << "Owner: " << focusedWindow->ownerName << std::endl;
            std::cout << "PID: " << focusedWindow->processId << std::endl;
            std::cout << "Position: " << formatPosition(focusedWindow->x, focusedWindow->y) << std::endl;
            std::cout << "Size: " << formatSize(focusedWindow->width, focusedWindow->height) << std::endl;
            std::cout << "State: " << getWindowStateIndicator(*focusedWindow) << std::endl;

            // Find and display workspace information
            for (const auto& workspace : workspaces) {
                if (workspace.id == focusedWindow->workspaceId) {
                    std::cout << "Workspace: " << formatWorkspaceInfo(workspace) << std::endl;
                    break;
                }
            }

            if (verbose_) {
                std::cout << "Handle: " << focusedWindow->handle << std::endl;
            }
        } else {
            std::cout << "No focused window found." << std::endl;
        }
    }
}

void CLI::displayCrossWorkspaceStatistics(const std::vector<WindowInfo>& allWindows, const std::vector<WorkspaceInfo>& workspaces) {
    if (outputFormat_ == "json") {
        std::cout << "{\n";
        std::cout << "  \"statistics\": {\n";
        std::cout << "    \"totalWindows\": " << allWindows.size() << ",\n";
        std::cout << "    \"totalWorkspaces\": " << workspaces.size() << ",\n";

        // Calculate statistics
        size_t visibleWindows = 0;
        size_t minimizedWindows = 0;
        size_t focusedWindows = 0;
        std::map<std::string, size_t> windowsByWorkspace;

        for (const auto& window : allWindows) {
            if (window.isVisible) visibleWindows++;
            if (window.isMinimized) minimizedWindows++;
            if (window.isFocused) focusedWindows++;
            windowsByWorkspace[window.workspaceId]++;
        }

        std::cout << "    \"visibleWindows\": " << visibleWindows << ",\n";
        std::cout << "    \"minimizedWindows\": " << minimizedWindows << ",\n";
        std::cout << "    \"focusedWindows\": " << focusedWindows << ",\n";
        std::cout << "    \"windowsByWorkspace\": {\n";

        bool firstWorkspace = true;
        for (const auto& workspace : workspaces) {
            if (!firstWorkspace) {
                std::cout << ",\n";
            }
            firstWorkspace = false;

            size_t count = windowsByWorkspace[workspace.id];
            std::cout << "      \"" << escapeJsonString(workspace.id) << "\": " << count;
        }

        std::cout << "\n    }\n";
        std::cout << "  }\n";
        std::cout << "}" << std::endl;
    } else {
        std::cout << "Cross-Workspace Statistics:" << std::endl;
        std::cout << "===========================" << std::endl;

        // Calculate statistics
        size_t visibleWindows = 0;
        size_t minimizedWindows = 0;
        size_t focusedWindows = 0;
        std::map<std::string, size_t> windowsByWorkspace;

        for (const auto& window : allWindows) {
            if (window.isVisible) visibleWindows++;
            if (window.isMinimized) minimizedWindows++;
            if (window.isFocused) focusedWindows++;
            windowsByWorkspace[window.workspaceId]++;
        }

        std::cout << "Total Windows: " << allWindows.size() << std::endl;
        std::cout << "Total Workspaces: " << workspaces.size() << std::endl;
        std::cout << "Visible Windows: " << visibleWindows << std::endl;
        std::cout << "Minimized Windows: " << minimizedWindows << std::endl;
        std::cout << "Focused Windows: " << focusedWindows << std::endl;
        std::cout << std::endl;

        std::cout << "Windows per Workspace:" << std::endl;
        for (const auto& workspace : workspaces) {
            size_t count = windowsByWorkspace[workspace.id];
            std::cout << "  " << formatWorkspaceInfo(workspace, false) << ": " << count << " windows" << std::endl;
        }
    }
}

std::string CLI::formatWorkspaceInfo(const WorkspaceInfo& workspace, bool includeIndex) {
    std::ostringstream oss;
    oss << workspace.name << " (ID: " << workspace.id;

    if (includeIndex) {
        oss << ", Index: " << workspace.index;
    }

    if (workspace.isCurrent) {
        oss << ", Current";
    }

    oss << ")";
    return oss.str();
}

} // namespace WindowManager