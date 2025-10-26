#pragma once

#include "../core/window_manager.hpp"
#include "../filters/search_query.hpp"
#include "../filters/filter_result.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include <thread>

namespace WindowManager {

/**
 * Interactive terminal interface using FTXUI
 * Provides real-time window filtering with live search
 */
class InteractiveUI {
public:
    explicit InteractiveUI(std::unique_ptr<WindowManager> windowManager);
    ~InteractiveUI();

    // Non-copyable, non-moveable (due to FTXUI ScreenInteractive)
    InteractiveUI(const InteractiveUI&) = delete;
    InteractiveUI& operator=(const InteractiveUI&) = delete;
    InteractiveUI(InteractiveUI&&) = delete;
    InteractiveUI& operator=(InteractiveUI&&) = delete;

    // Main interactive loop
    int run();

    // Configuration
    void setRefreshInterval(std::chrono::milliseconds interval);
    void setCaseSensitive(bool caseSensitive);

private:
    // Core components
    std::unique_ptr<WindowManager> windowManager_;
    ftxui::ScreenInteractive screen_;

    // UI state
    std::string searchInput_;
    bool caseSensitive_ = false;
    bool shouldExit_ = false;

    // Background refresh state
    std::atomic<bool> refreshEnabled_ = true;
    std::chrono::milliseconds refreshInterval_ = std::chrono::milliseconds(1000);
    std::thread refreshThread_;
    mutable std::mutex windowsMutex_;

    // UI display constants
    static constexpr size_t MAX_DISPLAYED_WINDOWS = 20;
    static constexpr size_t DEFAULT_WINDOW_TITLE_LENGTH = 60;
    static constexpr std::chrono::milliseconds REFRESH_SLEEP_INTERVAL{100};

    // Cached data
    std::vector<WindowInfo> allWindows_;
    FilterResult currentResult_;

    // Performance tracking
    std::chrono::steady_clock::time_point lastSearchTime_;
    bool performanceWarning_ = false;

    // UI Components
    ftxui::Component createMainComponent();
    ftxui::Component createSearchInput();
    ftxui::Component createWindowList();
    ftxui::Component createStatusBar();
    ftxui::Component createHelpText();

    // Content generators
    ftxui::Element renderWindowList();
    ftxui::Element renderWindow(const WindowInfo& window, int index);
    ftxui::Element renderStatusBar();
    ftxui::Element renderHelp();
    ftxui::Element renderPerformanceWarning();

    // Event handlers
    void onSearchInputChange();
    void onRefreshRequested();
    void onQuitRequested();

    // Background operations
    void startBackgroundRefresh();
    void stopBackgroundRefresh();
    void backgroundRefreshLoop();
    void updateWindowList();

    // Search and filtering
    void performSearch();
    SearchQuery createSearchQuery() const;

    // Utility methods
    std::string formatWindowTitle(const WindowInfo& window, size_t maxLength = DEFAULT_WINDOW_TITLE_LENGTH) const;
    std::string formatPosition(const WindowInfo& window) const;
    std::string formatSize(const WindowInfo& window) const;
    ftxui::Color getWindowColor(int index) const;
};

} // namespace WindowManager