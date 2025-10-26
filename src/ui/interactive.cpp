#include "interactive.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <algorithm>
#include <sstream>
#include <iomanip>

using namespace ftxui;

namespace WindowManager {

InteractiveUI::InteractiveUI(std::unique_ptr<WindowManager> windowManager)
    : windowManager_(std::move(windowManager))
    , screen_(ScreenInteractive::Fullscreen()) {

    if (!windowManager_) {
        throw std::invalid_argument("InteractiveUI requires a valid WindowManager");
    }

    // Initialize with empty search
    performSearch();
}

InteractiveUI::~InteractiveUI() {
    stopBackgroundRefresh();
}

int InteractiveUI::run() {
    // Start background refresh
    startBackgroundRefresh();

    // Create main UI component
    auto mainComponent = createMainComponent();

    // Start the interactive loop
    screen_.Loop(mainComponent);

    // Clean up
    stopBackgroundRefresh();

    return shouldExit_ ? 0 : 1;
}

void InteractiveUI::setRefreshInterval(std::chrono::milliseconds interval) {
    refreshInterval_ = interval;
}

void InteractiveUI::setCaseSensitive(bool caseSensitive) {
    caseSensitive_ = caseSensitive;
    performSearch(); // Re-search with new setting
}

Component InteractiveUI::createMainComponent() {
    // Create search input component
    auto searchComponent = Input(&searchInput_, "Enter search keyword...");

    // Add search input event handler
    searchComponent |= CatchEvent([this](Event event) {
        if (event.is_character() || event == Event::Backspace || event == Event::Delete) {
            // Delay search slightly to avoid too many updates while typing
            std::thread([this]() {
                std::this_thread::sleep_for(REFRESH_SLEEP_INTERVAL);
                onSearchInputChange();
            }).detach();
            return false; // Let the input component handle the event first
        }

        if (event == Event::F5) {
            onRefreshRequested();
            return true;
        }

        if (event == Event::Escape || (event.is_character() && (event.character() == "q" || event.character() == "Q"))) {
            onQuitRequested();
            return true;
        }

        if (event.is_character() && event.character() == "c") {
            caseSensitive_ = !caseSensitive_;
            performSearch();
            return true;
        }

        return false;
    });

    // Create the main container
    auto container = Container::Vertical({
        searchComponent
    });

    // Add renderer for the complete UI
    auto renderer = Renderer(container, [this]() {
        return vbox({
            // Header
            text("Window List and Filter Program - Interactive Mode") | bold | center,
            separator(),

            // Search box
            hbox({
                text("Search: "),
                filler(),
                text(caseSensitive_ ? "[Case Sensitive]" : "[Case Insensitive]") | dim,
            }),

            // Search input (this will be rendered by the input component)
            hbox({
                text("🔍 "),
                searchInput_.empty() ?
                    text("Enter search keyword...") | dim :
                    text(searchInput_),
                filler(),
            }) | border,

            separator(),

            // Window list
            renderWindowList() | flex,

            separator(),

            // Status bar
            renderStatusBar(),

            // Performance warning if needed
            performanceWarning_ ? renderPerformanceWarning() : text(""),

            separator(),

            // Help text
            renderHelp(),
        });
    });

    return renderer;
}

Component InteractiveUI::createSearchInput() {
    return Input(&searchInput_, "Enter search keyword...");
}

Element InteractiveUI::renderWindowList() {
    std::lock_guard<std::mutex> lock(windowsMutex_);

    if (currentResult_.windows.empty()) {
        if (searchInput_.empty()) {
            return vbox({
                text("No windows found") | center,
                text("Press F5 to refresh") | center | dim,
            }) | center | flex;
        } else {
            return vbox({
                text("No windows match '" + searchInput_ + "'") | center,
                text("Tips:") | center,
                text("• Try a shorter or more general keyword") | center | dim,
                text("• Press F5 to refresh window list") | center | dim,
                text("• Press 'c' to toggle case sensitivity") | center | dim,
            }) | center | flex;
        }
    }

    Elements windowElements;

    // Add header
    windowElements.push_back(
        hbox({
            text("Windows (" + std::to_string(currentResult_.filteredCount) +
                 " of " + std::to_string(currentResult_.totalCount) + ")") | bold,
            filler(),
            text("Search: " + std::to_string(currentResult_.searchTime.count()) + "ms") | dim,
        })
    );

    windowElements.push_back(separator());

    // Add windows
    for (size_t i = 0; i < currentResult_.windows.size() && i < MAX_DISPLAYED_WINDOWS; ++i) {
        windowElements.push_back(renderWindow(currentResult_.windows[i], static_cast<int>(i + 1)));
    }

    // Add "more" indicator if there are many windows
    if (currentResult_.windows.size() > MAX_DISPLAYED_WINDOWS) {
        windowElements.push_back(
            text("... and " + std::to_string(currentResult_.windows.size() - MAX_DISPLAYED_WINDOWS) + " more windows") | dim | center
        );
    }

    return vbox(windowElements) | flex;
}

Element InteractiveUI::renderWindow(const WindowInfo& window, int index) {
    auto titleColor = getWindowColor(index);

    return vbox({
        hbox({
            text("[" + std::to_string(index) + "] ") | color(Color::Blue),
            text(window.ownerName) | color(titleColor) | bold,
            window.title.empty() ? text("") : text(" - " + formatWindowTitle(window)) | color(titleColor),
            filler(),
            window.isVisible ? text("") : text("[Hidden]") | color(Color::Red) | dim,
        }),
        hbox({
            text("    Position: " + formatPosition(window)) | dim,
            text("  Size: " + formatSize(window)) | dim,
            text("  PID: " + std::to_string(window.processId)) | dim,
            filler(),
        }),
    });
}

Element InteractiveUI::renderStatusBar() {
    auto now = std::chrono::steady_clock::now();
    auto timeSinceRefresh = std::chrono::duration_cast<std::chrono::seconds>(now - lastSearchTime_);

    return hbox({
        text("Status: "),
        refreshEnabled_ ? text("Auto-refresh ON") | color(Color::Green) : text("Auto-refresh OFF") | color(Color::Red),
        text(" | "),
        text("Last refresh: " + std::to_string(timeSinceRefresh.count()) + "s ago") | dim,
        filler(),
        text("F5: Refresh | C: Toggle case | ESC/Q: Quit") | dim,
    });
}

Element InteractiveUI::renderHelp() {
    return vbox({
        text("Controls:") | bold,
        text("  Type to search windows in real-time"),
        text("  F5 - Manual refresh"),
        text("  C - Toggle case sensitivity"),
        text("  ESC or Q - Quit"),
    }) | dim;
}

Element InteractiveUI::renderPerformanceWarning() {
    return hbox({
        text("⚠ Warning: ") | color(Color::Yellow),
        text("Search is taking longer than 1 second. Consider using shorter keywords.") | color(Color::Yellow),
    });
}

void InteractiveUI::onSearchInputChange() {
    performSearch();
    screen_.PostEvent(Event::Custom); // Trigger screen refresh
}

void InteractiveUI::onRefreshRequested() {
    updateWindowList();
    performSearch();
    screen_.PostEvent(Event::Custom); // Trigger screen refresh
}

void InteractiveUI::onQuitRequested() {
    shouldExit_ = true;
    screen_.ExitLoopClosure()();
}

void InteractiveUI::startBackgroundRefresh() {
    refreshEnabled_ = true;
    refreshThread_ = std::thread(&InteractiveUI::backgroundRefreshLoop, this);
}

void InteractiveUI::stopBackgroundRefresh() {
    refreshEnabled_ = false;
    if (refreshThread_.joinable()) {
        refreshThread_.join();
    }
}

void InteractiveUI::backgroundRefreshLoop() {
    while (refreshEnabled_) {
        std::this_thread::sleep_for(refreshInterval_);

        if (!refreshEnabled_) break;

        updateWindowList();
        performSearch();

        // Post event to refresh the screen
        if (refreshEnabled_) {
            screen_.PostEvent(Event::Custom);
        }
    }
}

void InteractiveUI::updateWindowList() {
    try {
        std::lock_guard<std::mutex> lock(windowsMutex_);
        allWindows_ = windowManager_->getAllWindows();
    } catch (const std::exception&) {
        // Silently handle errors in background refresh
        // The user can manually refresh if needed
    }
}

void InteractiveUI::performSearch() {
    auto startTime = std::chrono::steady_clock::now();

    try {
        std::lock_guard<std::mutex> lock(windowsMutex_);

        auto query = createSearchQuery();
        currentResult_ = windowManager_->searchWindows(query);

        lastSearchTime_ = startTime;
        performanceWarning_ = !currentResult_.meetsPerformanceTarget();

    } catch (const std::exception&) {
        // Create empty result on error
        auto query = createSearchQuery();
        currentResult_ = windowManager_->getEmptyResult(query);
        performanceWarning_ = false;
    }
}

SearchQuery InteractiveUI::createSearchQuery() const {
    return SearchQuery(searchInput_, SearchField::Both, caseSensitive_, false);
}

std::string InteractiveUI::formatWindowTitle(const WindowInfo& window, size_t maxLength) const {
    if (window.title.length() <= maxLength) {
        return window.title;
    }
    return window.title.substr(0, maxLength - 3) + "...";
}

std::string InteractiveUI::formatPosition(const WindowInfo& window) const {
    return "(" + std::to_string(window.x) + ", " + std::to_string(window.y) + ")";
}

std::string InteractiveUI::formatSize(const WindowInfo& window) const {
    return std::to_string(window.width) + "x" + std::to_string(window.height);
}

Color InteractiveUI::getWindowColor(int index) const {
    const Color colors[] = {
        Color::Default,
        Color::Cyan,
        Color::Green,
        Color::Yellow,
        Color::Magenta,
        Color::Blue,
    };

    return colors[index % (sizeof(colors) / sizeof(colors[0]))];
}

} // namespace WindowManager