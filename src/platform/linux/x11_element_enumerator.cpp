#include "x11_element_enumerator.hpp"

#ifdef LINUX_PLATFORM

#include <sstream>

namespace WindowManager {

X11ElementEnumerator::X11ElementEnumerator()
    : display_(nullptr)
    , x11Connected_(false) {
    initializeX11();
}

X11ElementEnumerator::~X11ElementEnumerator() {
    cleanup();
}

bool X11ElementEnumerator::initializeX11() {
    display_ = XOpenDisplay(nullptr);
    if (!display_) {
        return false;
    }

    x11Connected_ = true;

    // Initialize common atoms
    atomWMName_ = XInternAtom(display_, "WM_NAME", False);
    atomWMClass_ = XInternAtom(display_, "WM_CLASS", False);
    atomNetWMName_ = XInternAtom(display_, "_NET_WM_NAME", False);
    atomNetWMWindowType_ = XInternAtom(display_, "_NET_WM_WINDOW_TYPE", False);

    return true;
}

void X11ElementEnumerator::cleanup() {
    if (display_) {
        XCloseDisplay(display_);
        display_ = nullptr;
    }
    x11Connected_ = false;
}

ElementEnumerationResult X11ElementEnumerator::enumerateElements(const std::string& windowHandle) {
    auto start = std::chrono::steady_clock::now();
    ElementEnumerationResult result(windowHandle);

    if (!x11Connected_) {
        result.success = false;
        result.errorMessage = "X11 connection not available";
        return result;
    }

    // Check cache first
    if (isCacheValid(windowHandle)) {
        result.elements = getCachedElements(windowHandle);
        result.totalElementCount = result.elements.size();
        result.filteredElementCount = result.elements.size();
        result.success = true;
        result.supportsElementEnumeration = true;
        result.hasAccessibilityPermissions = true;
        auto end = std::chrono::steady_clock::now();
        result.enumerationTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        return result;
    }

    try {
        Window window = stringToHandle(windowHandle);
        if (window == 0) {
            result.success = false;
            result.errorMessage = "Invalid window handle";
            return result;
        }

        // Get window title
        result.windowTitle = getWindowName(window);

        // Basic X11 element enumeration (limited functionality)
        result.elements = enumerateBasicElements(window, windowHandle);
        result.totalElementCount = result.elements.size();
        result.filteredElementCount = result.elements.size();
        result.success = true;
        result.supportsElementEnumeration = true;
        result.hasAccessibilityPermissions = true;

        // Update cache
        updateCache(windowHandle, result.elements);

    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = e.what();
    }

    auto end = std::chrono::steady_clock::now();
    result.enumerationTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    updateEnumerationTime(start, end);

    return result;
}

ElementEnumerationResult X11ElementEnumerator::searchElements(const std::string& windowHandle,
                                                             const ElementSearchQuery& query) {
    // Get all elements first
    ElementEnumerationResult allElements = enumerateElements(windowHandle);
    if (!allElements.success) {
        return allElements;
    }

    // Filter elements based on query
    ElementEnumerationResult result(windowHandle);
    result.windowTitle = allElements.windowTitle;
    result.totalElementCount = allElements.totalElementCount;
    result.success = true;
    result.supportsElementEnumeration = true;
    result.hasAccessibilityPermissions = true;
    result.enumerationTime = allElements.enumerationTime;

    for (const auto& element : allElements.elements) {
        if (query.matches(element)) {
            result.elements.push_back(element);
        }
    }

    result.filteredElementCount = result.elements.size();
    return result;
}

std::optional<UIElement> X11ElementEnumerator::getElementInfo(const std::string& elementHandle) {
    return std::nullopt; // Simplified implementation
}

bool X11ElementEnumerator::isElementValid(const std::string& elementHandle) {
    return false; // Simplified implementation
}

bool X11ElementEnumerator::supportsElementEnumeration(const std::string& windowHandle) {
    return x11Connected_;
}

bool X11ElementEnumerator::hasElementAccessPermissions() const {
    return x11Connected_;
}

void X11ElementEnumerator::clearElementCache(const std::string& windowHandle) {
    elementCache_.erase(windowHandle);
    cacheTimestamps_.erase(windowHandle);
}

void X11ElementEnumerator::clearAllElementCaches() {
    elementCache_.clear();
    cacheTimestamps_.clear();
}

std::chrono::milliseconds X11ElementEnumerator::getLastEnumerationTime() const {
    return lastEnumerationDuration_;
}

std::string X11ElementEnumerator::getPlatformInfo() const {
    return "Linux X11 (Basic Implementation)";
}

// Helper methods
std::vector<UIElement> X11ElementEnumerator::enumerateBasicElements(Window window, const std::string& windowHandle) {
    std::vector<UIElement> elements;

    // Create a basic window element representation
    UIElement windowElement;
    windowElement.handle = windowHandle + "-window";
    windowElement.parentWindowHandle = windowHandle;
    windowElement.type = ElementType::Window;
    windowElement.name = getWindowName(window);
    windowElement.isVisible = isWindowVisible(window);
    windowElement.isEnabled = true;
    windowElement.discoveredAt = std::chrono::steady_clock::now();

    // Get window geometry
    XWindowAttributes attrs;
    if (XGetWindowAttributes(display_, window, &attrs) == Success) {
        windowElement.x = attrs.x;
        windowElement.y = attrs.y;
        windowElement.width = attrs.width;
        windowElement.height = attrs.height;
    }

    elements.push_back(windowElement);

    // Try to enumerate child windows as basic elements
    auto childElements = findChildWindows(window, windowHandle);
    elements.insert(elements.end(), childElements.begin(), childElements.end());

    return elements;
}

std::vector<UIElement> X11ElementEnumerator::findChildWindows(Window window, const std::string& windowHandle) {
    std::vector<UIElement> elements;

    Window root, parent;
    Window* children = nullptr;
    unsigned int nchildren;

    if (XQueryTree(display_, window, &root, &parent, &children, &nchildren) == Success) {
        for (unsigned int i = 0; i < nchildren; ++i) {
            UIElement childElement;
            childElement.handle = windowHandle + "-child-" + std::to_string(i);
            childElement.parentWindowHandle = windowHandle;
            childElement.type = guessElementTypeFromX11(children[i]);
            childElement.name = getWindowName(children[i]);
            childElement.isVisible = isWindowVisible(children[i]);
            childElement.isEnabled = true;
            childElement.discoveredAt = std::chrono::steady_clock::now();

            // Get child geometry
            XWindowAttributes attrs;
            if (XGetWindowAttributes(display_, children[i], &attrs) == Success) {
                childElement.x = attrs.x;
                childElement.y = attrs.y;
                childElement.width = attrs.width;
                childElement.height = attrs.height;
            }

            elements.push_back(childElement);
        }

        if (children) {
            XFree(children);
        }
    }

    return elements;
}

// Utility methods
std::string X11ElementEnumerator::handleToString(Window window) {
    return std::to_string(window);
}

Window X11ElementEnumerator::stringToHandle(const std::string& handleStr) {
    try {
        return std::stoul(handleStr);
    } catch (...) {
        return 0;
    }
}

ElementType X11ElementEnumerator::guessElementTypeFromX11(Window window) {
    // Very basic type guessing based on window properties
    std::string className = getWindowClass(window);

    if (className.find("button") != std::string::npos ||
        className.find("Button") != std::string::npos) {
        return ElementType::Button;
    }
    if (className.find("text") != std::string::npos ||
        className.find("edit") != std::string::npos) {
        return ElementType::TextField;
    }
    if (className.find("menu") != std::string::npos) {
        return ElementType::Menu;
    }

    return ElementType::Pane; // Default for unknown child windows
}

bool X11ElementEnumerator::isWindowVisible(Window window) {
    XWindowAttributes attrs;
    if (XGetWindowAttributes(display_, window, &attrs) == Success) {
        return attrs.map_state == IsViewable;
    }
    return false;
}

std::string X11ElementEnumerator::getWindowName(Window window) {
    char* name = nullptr;
    if (XFetchName(display_, window, &name) == Success && name) {
        std::string result(name);
        XFree(name);
        return result;
    }
    return "Unknown";
}

std::string X11ElementEnumerator::getWindowClass(Window window) {
    XClassHint classHint;
    if (XGetClassHint(display_, window, &classHint) == Success) {
        std::string result = classHint.res_class ? classHint.res_class : "";
        if (classHint.res_name) XFree(classHint.res_name);
        if (classHint.res_class) XFree(classHint.res_class);
        return result;
    }
    return "";
}

// Cache management
bool X11ElementEnumerator::isCacheValid(const std::string& windowHandle) const {
    auto it = cacheTimestamps_.find(windowHandle);
    if (it == cacheTimestamps_.end()) {
        return false;
    }

    auto now = std::chrono::steady_clock::now();
    auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second);
    return age < CACHE_TIMEOUT;
}

void X11ElementEnumerator::updateCache(const std::string& windowHandle, const std::vector<UIElement>& elements) {
    elementCache_[windowHandle] = elements;
    cacheTimestamps_[windowHandle] = std::chrono::steady_clock::now();
}

std::vector<UIElement> X11ElementEnumerator::getCachedElements(const std::string& windowHandle) const {
    auto it = elementCache_.find(windowHandle);
    return it != elementCache_.end() ? it->second : std::vector<UIElement>();
}

} // namespace WindowManager

#endif // LINUX_PLATFORM