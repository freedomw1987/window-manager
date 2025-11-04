#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <chrono>

#include "core/window.hpp"
#include "core/enumerator.hpp"
#include "core/window_manager.hpp"
#include "core/exceptions.hpp"
#include "ui/cli.hpp"
#include "ui/interactive.hpp"
#include "filters/search_query.hpp"
#include "filters/filter_result.hpp"
#include "platform_config.h"

// Function declarations for different modes
int listWindows(bool verbose = false, const std::string& format = "text", bool showHandles = false, bool handlesOnly = false);
int searchWindows(const std::string& keyword, bool caseSensitive = false, bool verbose = false, const std::string& format = "text");
int focusWindow(const std::string& handle, bool verbose = false, const std::string& format = "text",
                bool allowWorkspaceSwitch = true, int timeout = 5);
int validateHandle(const std::string& handle, bool verbose = false, const std::string& format = "text");
int interactiveMode(const std::string& format = "text");
void printUsage(const char* programName);
void printVersion();
void printPlatformSpecificHelp();
void printDetailedErrorGuidance(const std::string& errorType);

int main(int argc, char* argv[]) {
    try {
        // Parse command line arguments
        std::vector<std::string> args(argv, argv + argc);

        if (args.size() < 2) {
            printUsage(argv[0]);
            return 1;
        }

        std::string command = args[1];

        // Handle help and version flags
        if (command == "--help" || command == "-h") {
            printUsage(argv[0]);
            return 0;
        }

        if (command == "--version" || command == "-v") {
            printVersion();
            return 0;
        }

        if (command == "--platform-help") {
            printPlatformSpecificHelp();
            return 0;
        }

        // Parse options
        bool verbose = false;
        bool caseSensitive = false;
        std::string format = "text";

        for (size_t i = 2; i < args.size(); ++i) {
            if (args[i] == "--verbose") {
                verbose = true;
            } else if (args[i] == "--case-sensitive" || args[i] == "-c") {
                caseSensitive = true;
            } else if (args[i] == "--format" || args[i] == "-f") {
                if (i + 1 < args.size()) {
                    format = args[++i];
                    if (format != "text" && format != "json") {
                        std::cerr << "Error: Invalid format '" << format << "'. Use 'text' or 'json'.\n";
                        return 1;
                    }
                } else {
                    std::cerr << "Error: --format requires an argument (text|json)\n";
                    return 1;
                }
            }
        }

        // Execute commands
        if (command == "list") {
            // T040-T041: Parse list-specific options
            bool showHandles = false;
            bool handlesOnly = false;

            for (size_t i = 2; i < args.size(); ++i) {
                if (args[i] == "--show-handles") {
                    showHandles = true;
                } else if (args[i] == "--handles-only") {
                    handlesOnly = true;
                }
                // Other options like --verbose and --format are already parsed above
            }

            return listWindows(verbose, format, showHandles, handlesOnly);
        } else if (command == "search") {
            if (args.size() < 3) {
                std::cerr << "Error: search command requires a keyword\n";
                printUsage(argv[0]);
                return 1;
            }
            std::string keyword = args[2];
            return searchWindows(keyword, caseSensitive, verbose, format);
        } else if (command == "focus") {
            if (args.size() < 3) {
                std::cerr << "Error: focus command requires a window handle\n";
                printUsage(argv[0]);
                return 1;
            }
            std::string handle = args[2];

            // Parse focus-specific options
            bool allowWorkspaceSwitch = true;  // Default: allow workspace switching (User Story 2)
            int timeout = 5;  // Default timeout in seconds

            for (size_t i = 3; i < args.size(); ++i) {
                if (args[i] == "--no-workspace-switch") {
                    allowWorkspaceSwitch = false;
                } else if (args[i] == "--timeout") {
                    if (i + 1 < args.size()) {
                        try {
                            timeout = std::stoi(args[++i]);
                        } catch (const std::exception&) {
                            std::cerr << "Error: Invalid timeout value\n";
                            return 1;
                        }
                    } else {
                        std::cerr << "Error: --timeout requires a value\n";
                        return 1;
                    }
                }
            }

            return focusWindow(handle, verbose, format, allowWorkspaceSwitch, timeout);
        } else if (command == "validate-handle") {
            if (args.size() < 3) {
                std::cerr << "Error: validate-handle command requires a window handle\n";
                printUsage(argv[0]);
                return 1;
            }
            std::string handle = args[2];
            return validateHandle(handle, verbose, format);
        } else if (command == "interactive") {
            return interactiveMode(format);
        } else {
            std::cerr << "Error: Unknown command '" << command << "'\n";
            printUsage(argv[0]);
            return 1;
        }

    } catch (const WindowManager::PlatformNotSupportedException& e) {
        std::cerr << "Platform Error: " << e.what() << std::endl;
        std::cerr << "This application supports Windows, Linux (X11), and macOS only." << std::endl;
        std::cerr << "\nPlatform Information:" << std::endl;
        std::cerr << "Current platform: " << WM_PLATFORM_NAME << std::endl;
        printDetailedErrorGuidance("platform");
        return 2;
    } catch (const WindowManager::PermissionDeniedException& e) {
        std::cerr << "Permission Error: " << e.what() << std::endl;
        printDetailedErrorGuidance("permission");
        return 3;
    } catch (const WindowManager::WindowEnumerationException& e) {
        std::cerr << "Enumeration Error: " << e.what() << std::endl;
        printDetailedErrorGuidance("enumeration");
        return 4;
    } catch (const WindowManager::WindowManagerException& e) {
        std::cerr << "Window Manager Error: " << e.what() << std::endl;
        return 5;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected Error: " << e.what() << std::endl;
        std::cerr << "Please report this error to the developers." << std::endl;
        return 99;
    }
}

int listWindows(bool verbose, const std::string& format, bool showHandles, bool handlesOnly) {
    try {
        // Create window manager
        auto windowManager = WindowManager::WindowManager::create();

        // Set up CLI
        WindowManager::CLI cli;
        cli.setOutputFormat(format);
        cli.setVerbose(verbose);

        // Get all windows
        auto start = std::chrono::steady_clock::now();
        auto windows = windowManager->getAllWindows();
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // Display windows based on options
        if (showHandles || handlesOnly) {
            cli.displayAllWindowsWithHandles(windows, handlesOnly);
        } else {
            cli.displayAllWindows(windows);
        }

        // Show performance stats if verbose
        if (verbose) {
            cli.displayPerformanceStats(duration, windows.size());
            cli.displayInfo("Platform: " + windowManager->getSystemInfo());

            // Validate performance requirements
            if (windowManager->meetsPerformanceRequirements()) {
                cli.displaySuccess("Performance requirements met");
            } else {
                cli.displayError("Performance requirements not met - enumeration took too long");
            }
        }

        return 0;

    } catch (const WindowManager::WindowManagerException& e) {
        std::cerr << "Window Manager Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

int searchWindows(const std::string& keyword, bool caseSensitive, bool verbose, const std::string& format) {
    try {
        // Create window manager
        auto windowManager = WindowManager::WindowManager::create();

        // Set up CLI
        WindowManager::CLI cli;
        cli.setOutputFormat(format);
        cli.setVerbose(verbose);

        // Create search query
        WindowManager::SearchQuery query(keyword, WindowManager::SearchField::Both, caseSensitive, false);

        if (verbose) {
            std::cerr << "Debug: Starting search for '" << keyword << "'" << std::endl;
            std::cerr << "Debug: Case sensitive: " << (caseSensitive ? "yes" : "no") << std::endl;
        }

        // Perform search
        auto start = std::chrono::steady_clock::now();
        auto result = windowManager->searchWindows(query);
        auto end = std::chrono::steady_clock::now();

        if (verbose) {
            auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cerr << "Debug: Search completed in " << totalTime.count() << "ms" << std::endl;
            std::cerr << "Debug: Found " << result.filteredCount << " matches out of " << result.totalCount << " total windows" << std::endl;
        }

        // Display results
        if (result.filteredCount > 0) {
            cli.displayFilteredResults(result);
        } else {
            cli.displayNoMatches(keyword);
        }

        // Show performance warning if needed
        if (!result.meetsPerformanceTarget()) {
            if (format == "text") {
                std::cerr << "\nWarning: Search took " << result.searchTime.count()
                          << "ms (exceeds 1 second performance target)" << std::endl;
            }
        }

        // Show additional verbose information
        if (verbose) {
            std::cerr << "\nVerbose Information:" << std::endl;
            std::cerr << "- Platform: " << windowManager->getSystemInfo() << std::endl;
            std::cerr << "- Performance meets requirements: " << (windowManager->meetsPerformanceRequirements() ? "yes" : "no") << std::endl;
            std::cerr << "- Cache enabled: " << (windowManager->isCachingEnabled() ? "yes" : "no") << std::endl;
        }

        return 0;

    } catch (const WindowManager::WindowManagerException& e) {
        std::cerr << "Window Manager Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

int focusWindow(const std::string& handle, bool verbose, const std::string& format,
                bool allowWorkspaceSwitch, int timeout) {
    // T043: Use timeout parameter for focus operations
    try {
        // Create window manager
        auto windowManager = WindowManager::WindowManager::create();

        // Set up CLI
        WindowManager::CLI cli;
        cli.setOutputFormat(format);
        cli.setVerbose(verbose);

        // Show progress if verbose
        if (verbose) {
            cli.displayFocusProgress(handle, "Validating window handle");
        }

        // T043: Validate handle with custom timeout
        if (!windowManager->validateHandleWithTimeout(handle, std::chrono::milliseconds(timeout * 1000))) {
            cli.displayFocusError(handle, "Handle validation failed or timed out",
                                "Check if the window handle is valid and accessible");
            return 1;
        }

        // Attempt to focus the window
        auto startTime = std::chrono::steady_clock::now();
        bool success = windowManager->focusWindowByHandle(handle, allowWorkspaceSwitch); // User Story 2: support workspace switching
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        if (success) {
            // Get window information for display
            auto windowInfo = windowManager->getWindowByHandle(handle);
            std::string title = windowInfo ? windowInfo->title : "";
            std::string workspace = windowInfo ? windowInfo->workspaceName : "";
            bool workspaceSwitched = windowInfo ? !windowInfo->isOnCurrentWorkspace : false;

            cli.displayFocusSuccess(handle, title, workspace, workspaceSwitched, duration);

            // Check performance requirements (SC-001/SC-002)
            if (verbose) {
                std::chrono::milliseconds threshold = workspaceSwitched ?
                    std::chrono::milliseconds(2000) :  // SC-002: < 2 seconds for cross-workspace
                    std::chrono::milliseconds(1000);   // SC-001: < 1 second for current workspace

                if (duration <= threshold) {
                    cli.displayInfo("Performance requirement met (" +
                                  std::to_string(threshold.count()) + "ms threshold)");
                } else {
                    cli.displayError("Performance requirement not met (took " +
                                   std::to_string(duration.count()) + "ms, threshold: " +
                                   std::to_string(threshold.count()) + "ms)");
                }
            }

            return 0;
        } else {
            // Focus failed - determine why and provide helpful error message
            if (!windowManager->validateHandle(handle)) {
                cli.displayFocusError(handle, "Invalid window handle",
                                    "Use 'window-manager list --show-handles' to see available windows");
            } else {
                auto windowInfo = windowManager->getWindowByHandle(handle);
                if (!windowInfo) {
                    cli.displayFocusError(handle, "Window not found",
                                        "Window may have been closed or is no longer available");
                } else if (!windowInfo->isOnCurrentWorkspace && !allowWorkspaceSwitch) {
                    cli.displayFocusError(handle, "Window is in different workspace",
                                        "Try without --no-workspace-switch option to allow workspace switching");
                } else {
                    cli.displayFocusError(handle, "Failed to focus window",
                                        "Window may be restricted or require elevated permissions");
                }
            }
            return 1;
        }

    } catch (const WindowManager::WindowManagerException& e) {
        std::cerr << "Window Manager Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

int validateHandle(const std::string& handle, bool verbose, const std::string& format) {
    try {
        // Create window manager
        auto windowManager = WindowManager::WindowManager::create();

        // Set up CLI
        WindowManager::CLI cli;
        cli.setOutputFormat(format);
        cli.setVerbose(verbose);

        // Show progress if verbose
        if (verbose) {
            cli.displayFocusProgress(handle, "Validating window handle format and existence");
        }

        auto startTime = std::chrono::steady_clock::now();

        // Validate the handle
        bool isValid = windowManager->validateHandle(handle);

        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        if (isValid) {
            // Get additional window information for validation
            auto windowInfo = windowManager->getWindowByHandle(handle);
            std::string reason = "Window exists and is accessible";

            if (windowInfo) {
                if (!windowInfo->focusable) {
                    reason = "Window exists but may not be focusable";
                } else if (windowInfo->requiresRestore) {
                    reason = "Window exists, focusable, but currently minimized";
                } else if (windowInfo->workspaceSwitchRequired) {
                    reason = "Window exists, focusable, but requires workspace switching";
                }
            }

            cli.displayHandleValidation(handle, true, reason);

            // Additional verbose information
            if (verbose && windowInfo) {
                cli.displayInfo("Window title: \"" + windowInfo->title + "\"");
                cli.displayInfo("Workspace: " + windowInfo->workspaceName);
                cli.displayInfo("Process: " + windowInfo->ownerName + " (PID: " + std::to_string(windowInfo->processId) + ")");
            }
        } else {
            std::string reason = "Handle format invalid or window not found";

            // Try to provide more specific error information
            if (handle.empty()) {
                reason = "Handle cannot be empty";
            } else if (handle.find_first_not_of("0123456789") != std::string::npos) {
                reason = "Handle must be numeric (platform-specific window ID)";
            } else {
                reason = "Window with this handle does not exist or is not accessible";
            }

            cli.displayHandleValidation(handle, false, reason);
        }

        // Check performance requirements (SC-003: < 0.5 seconds for validation)
        if (verbose) {
            if (duration <= std::chrono::milliseconds(500)) {
                cli.displayInfo("Validation performance requirement met (< 0.5 seconds)");
            } else {
                cli.displayError("Validation performance requirement not met (took " +
                               std::to_string(duration.count()) + "ms)");
            }
        }

        return isValid ? 0 : 1;

    } catch (const WindowManager::WindowManagerException& e) {
        std::cerr << "Window Manager Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

int interactiveMode(const std::string& format) {
    try {
        // Create window manager
        auto windowManager = WindowManager::WindowManager::create();

        // Create interactive UI
        WindowManager::InteractiveUI ui(std::move(windowManager));

        // Note: format parameter is ignored in interactive mode as it uses FTXUI
        if (format != "text") {
            std::cerr << "Note: Interactive mode uses terminal UI, ignoring format option.\n\n";
        }

        // Start interactive loop
        return ui.run();

    } catch (const WindowManager::WindowManagerException& e) {
        std::cerr << "Window Manager Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

void printUsage(const char* programName) {
    std::cout << "Window List and Filter Program\n";
    std::cout << "Usage: " << programName << " [options] <command> [args...]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  list                    List all windows\n";
    std::cout << "  search <keyword>        Search windows by keyword\n";
    std::cout << "  focus <handle>          Focus window by handle (with workspace switching)\n";
    std::cout << "  validate-handle <handle> Validate window handle format and existence\n";
    std::cout << "  interactive             Start interactive filtering mode\n\n";
    std::cout << "Options:\n";
    std::cout << "  --help, -h              Show this help message\n";
    std::cout << "  --version, -v           Show version information\n";
    std::cout << "  --platform-help         Show platform-specific setup information\n";
    std::cout << "  --format, -f <format>   Output format (text|json)\n";
    std::cout << "  --verbose               Enable verbose output\n";
    std::cout << "  --case-sensitive, -c    Enable case-sensitive search\n";
    std::cout << "  --no-workspace-switch   Prevent automatic workspace switching (focus command)\n";
    std::cout << "  --timeout <seconds>     Set operation timeout (focus command)\n";
    std::cout << "  --show-handles          Show window handles in list output\n";
    std::cout << "  --handles-only          Show only handles and titles (compact format)\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << programName << " list\n";
    std::cout << "  " << programName << " list --format json --verbose\n";
    std::cout << "  " << programName << " list --show-handles\n";
    std::cout << "  " << programName << " list --handles-only\n";
    std::cout << "  " << programName << " search chrome\n";
    std::cout << "  " << programName << " search \"Google Chrome\" --case-sensitive\n";
    std::cout << "  " << programName << " focus 12345\n";
    std::cout << "  " << programName << " focus 12345 --verbose\n";
    std::cout << "  " << programName << " focus 12345 --no-workspace-switch\n";
    std::cout << "  " << programName << " focus 12345 --timeout 10\n";
    std::cout << "  " << programName << " validate-handle 12345\n";
    std::cout << "  " << programName << " validate-handle 12345 --format json\n";
    std::cout << "  " << programName << " interactive\n";
}

void printVersion() {
    std::cout << "Window List and Filter Program v1.0.0\n";
    std::cout << "Platform: " << WM_PLATFORM_NAME << std::endl;
    std::cout << "Built with C++17 and CMake\n";
}

void printPlatformSpecificHelp() {
    std::cout << "\nPlatform-Specific Information:\n";
    std::cout << "============================\n";

#ifdef WINDOWS_PLATFORM
    std::cout << "Windows Platform:\n";
    std::cout << "- Uses Win32 API for window enumeration\n";
    std::cout << "- Requires Windows Vista or later\n";
    std::cout << "- May need administrator privileges for some system windows\n";
    std::cout << "- If permission errors occur, try running as administrator\n";
#elif defined(MACOS_PLATFORM)
    std::cout << "macOS Platform:\n";
    std::cout << "- Uses Core Graphics and Accessibility APIs\n";
    std::cout << "- Requires macOS 10.12 (Sierra) or later\n";
    std::cout << "- IMPORTANT: Accessibility permissions must be granted\n";
    std::cout << "  1. Open System Preferences > Security & Privacy > Privacy\n";
    std::cout << "  2. Select 'Accessibility' from the left panel\n";
    std::cout << "  3. Click the lock to make changes (admin password required)\n";
    std::cout << "  4. Add this application to the list and check the box\n";
    std::cout << "  5. Restart this application after granting permission\n";
#elif defined(LINUX_PLATFORM)
    std::cout << "Linux Platform:\n";
    std::cout << "- Uses X11 API for window enumeration\n";
    std::cout << "- Requires X11 server to be running\n";
    std::cout << "- DISPLAY environment variable must be set correctly\n";
    std::cout << "- Does not work with Wayland compositors directly\n";
    std::cout << "- Common issues:\n";
    std::cout << "  * SSH: Use 'ssh -X' or 'ssh -Y' for X11 forwarding\n";
    std::cout << "  * Wayland: Try running under XWayland compatibility layer\n";
    std::cout << "  * Check: echo $DISPLAY (should show something like :0 or :0.0)\n";
#else
    std::cout << "Unknown Platform: This platform is not officially supported\n";
#endif
    std::cout << "\n";
}

void printDetailedErrorGuidance(const std::string& errorType) {
    std::cout << "\nDetailed Troubleshooting:\n";
    std::cout << "========================\n";

    if (errorType == "platform") {
        std::cout << "This error indicates that your operating system is not supported.\n";
        std::cout << "Supported platforms:\n";
        std::cout << "- Windows (Vista and later)\n";
        std::cout << "- macOS (Sierra 10.12 and later)\n";
        std::cout << "- Linux (with X11)\n\n";
        std::cout << "If you're on a supported platform but still seeing this error,\n";
        std::cout << "it may be a build or compilation issue.\n";

    } else if (errorType == "permission") {
        std::cout << "Permission errors occur when the application cannot access window information.\n\n";

#ifdef WINDOWS_PLATFORM
        std::cout << "Windows Solutions:\n";
        std::cout << "1. Right-click the application and select 'Run as administrator'\n";
        std::cout << "2. Check Windows Security settings\n";
        std::cout << "3. Ensure no antivirus is blocking the application\n";
        std::cout << "4. Try running from an elevated command prompt\n";
#elif defined(MACOS_PLATFORM)
        std::cout << "macOS Solutions:\n";
        std::cout << "1. Grant Accessibility permissions (see help above)\n";
        std::cout << "2. System Preferences > Security & Privacy > Privacy > Accessibility\n";
        std::cout << "3. Add this application and enable it\n";
        std::cout << "4. Restart the application after granting permissions\n";
        std::cout << "5. If still failing, try running with sudo (not recommended)\n";
#elif defined(LINUX_PLATFORM)
        std::cout << "Linux Solutions:\n";
        std::cout << "1. Ensure X11 is running: ps aux | grep Xorg\n";
        std::cout << "2. Check DISPLAY variable: echo $DISPLAY\n";
        std::cout << "3. Try: xhost +local: (adds local permissions)\n";
        std::cout << "4. For SSH: use ssh -X or ssh -Y\n";
        std::cout << "5. Install required X11 development packages\n";
#endif

    } else if (errorType == "enumeration") {
        std::cout << "Window enumeration failed. This can happen due to:\n\n";
        std::cout << "Common Causes:\n";
        std::cout << "1. System resource constraints (low memory, high CPU)\n";
        std::cout << "2. Platform-specific API limitations\n";
        std::cout << "3. Corrupted window manager state\n";
        std::cout << "4. Security software interference\n\n";

        std::cout << "Solutions to Try:\n";
        std::cout << "1. Close unnecessary applications to free up resources\n";
        std::cout << "2. Restart your window manager/desktop environment\n";
        std::cout << "3. Try running with --verbose flag for more details\n";
        std::cout << "4. Check system logs for related errors\n";
        std::cout << "5. Reboot if the problem persists\n";
    }

    std::cout << "\nFor additional help:\n";
    std::cout << "- Run with --verbose flag for detailed information\n";
    std::cout << "- Check the README.md file for platform-specific setup\n";
    std::cout << "- Report persistent issues to the developers\n";
    std::cout << "\n";
}