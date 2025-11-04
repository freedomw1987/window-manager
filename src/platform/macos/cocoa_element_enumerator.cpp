#include "cocoa_element_enumerator.hpp"

#ifdef MACOS_PLATFORM

#include <sstream>
#include <iostream>

namespace WindowManager {

CocoaElementEnumerator::CocoaElementEnumerator() {
    // Constructor implementation
}

ElementEnumerationResult CocoaElementEnumerator::enumerateElements(const std::string& windowHandle) {
    auto start = std::chrono::steady_clock::now();
    ElementEnumerationResult result(windowHandle);

    // Check accessibility permissions first
    if (!hasElementAccessPermissions()) {
        result.success = false;
        result.errorMessage = "Accessibility permissions required";
        result.hasAccessibilityPermissions = false;
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
        CGWindowID windowId = stringToHandle(windowHandle);
        if (windowId == 0) {
            result.success = false;
            result.errorMessage = "Invalid window handle";
            return result;
        }

        // Enhanced implementation - attempt to enumerate actual UI elements using Accessibility API
        std::vector<UIElement> elements;

        // Get window information to determine application type
        CFArrayRef windowArray = CGWindowListCopyWindowInfo(kCGWindowListOptionAll, kCGNullWindowID);
        std::string ownerName = "Unknown";
        std::string windowTitle = "";

        if (windowArray) {
            CFIndex windowCount = CFArrayGetCount(windowArray);
            for (CFIndex i = 0; i < windowCount; i++) {
                CFDictionaryRef windowInfo = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(windowArray, i));

                CFNumberRef windowIdRef = static_cast<CFNumberRef>(CFDictionaryGetValue(windowInfo, kCGWindowNumber));
                if (windowIdRef) {
                    CGWindowID currentWindowId;
                    CFNumberGetValue(windowIdRef, kCGWindowIDCFNumberType, &currentWindowId);

                    if (currentWindowId == windowId) {
                        // Found our window, get owner information
                        CFStringRef ownerRef = static_cast<CFStringRef>(CFDictionaryGetValue(windowInfo, kCGWindowOwnerName));
                        if (ownerRef) {
                            char ownerBuffer[256];
                            CFStringGetCString(ownerRef, ownerBuffer, sizeof(ownerBuffer), kCFStringEncodingUTF8);
                            ownerName = ownerBuffer;
                        }

                        CFStringRef titleRef = static_cast<CFStringRef>(CFDictionaryGetValue(windowInfo, kCGWindowName));
                        if (titleRef) {
                            char titleBuffer[256];
                            CFStringGetCString(titleRef, titleBuffer, sizeof(titleBuffer), kCFStringEncodingUTF8);
                            windowTitle = titleBuffer;
                        }
                        break;
                    }
                }
            }
            CFRelease(windowArray);
        }

        // Use real Accessibility API to enumerate actual UI elements
        AXUIElementRef windowElement = getWindowElement(windowHandle);
        if (windowElement) {
            elements = traverseElementTree(windowElement, windowHandle);
            result.windowTitle = windowTitle.empty() ? (ownerName + " Window") : windowTitle;
        } else {
            // Fallback: if we can't get accessibility elements, provide basic info
            result.success = false;
            result.errorMessage = "Unable to access window elements - may require accessibility permissions or window may not support element enumeration";
            result.windowTitle = ownerName + " Window";
        }

        result.elements = elements;
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

ElementEnumerationResult CocoaElementEnumerator::searchElements(const std::string& windowHandle,
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

std::optional<UIElement> CocoaElementEnumerator::getElementInfo(const std::string& /* elementHandle */) {
    return std::nullopt; // Simplified implementation
}

bool CocoaElementEnumerator::isElementValid(const std::string& /* elementHandle */) {
    return false; // Simplified implementation
}

bool CocoaElementEnumerator::supportsElementEnumeration(const std::string& /* windowHandle */) {
    return hasElementAccessPermissions();
}

bool CocoaElementEnumerator::hasElementAccessPermissions() const {
    return checkAccessibilityPermissions();
}

void CocoaElementEnumerator::clearElementCache(const std::string& windowHandle) {
    elementCache_.erase(windowHandle);
    cacheTimestamps_.erase(windowHandle);
}

void CocoaElementEnumerator::clearAllElementCaches() {
    elementCache_.clear();
    cacheTimestamps_.clear();
}

std::chrono::milliseconds CocoaElementEnumerator::getLastEnumerationTime() const {
    return lastEnumerationDuration_;
}

std::string CocoaElementEnumerator::getPlatformInfo() const {
    return "macOS Accessibility API (Simplified)";
}

// Helper methods
std::string CocoaElementEnumerator::handleToString(CGWindowID windowId) {
    return std::to_string(windowId);
}

CGWindowID CocoaElementEnumerator::stringToHandle(const std::string& handleStr) {
    try {
        // Handle both decimal and hexadecimal formats
        if (handleStr.find_first_not_of("0123456789") != std::string::npos) {
            // Contains non-decimal characters, try hexadecimal
            return std::stoul(handleStr, nullptr, 16);
        } else {
            // All decimal digits, parse as decimal
            return std::stoul(handleStr);
        }
    } catch (...) {
        return 0;
    }
}

// Cache management
bool CocoaElementEnumerator::isCacheValid(const std::string& windowHandle) const {
    auto it = cacheTimestamps_.find(windowHandle);
    if (it == cacheTimestamps_.end()) {
        return false;
    }

    auto now = std::chrono::steady_clock::now();
    auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second);
    return age < CACHE_TIMEOUT;
}

void CocoaElementEnumerator::updateCache(const std::string& windowHandle, const std::vector<UIElement>& elements) {
    elementCache_[windowHandle] = elements;
    cacheTimestamps_[windowHandle] = std::chrono::steady_clock::now();
}

std::vector<UIElement> CocoaElementEnumerator::getCachedElements(const std::string& windowHandle) const {
    auto it = elementCache_.find(windowHandle);
    return it != elementCache_.end() ? it->second : std::vector<UIElement>();
}

bool CocoaElementEnumerator::checkAccessibilityPermissions() const {
    // Check if accessibility is enabled for this application
    return AXIsProcessTrusted();
}

// Application-specific element creation methods

void CocoaElementEnumerator::createWordElements(std::vector<UIElement>& elements, const std::string& windowHandle, const std::string& /* windowTitle */) {
    // Create typical Microsoft Word elements

    // Main document window
    UIElement documentWindow;
    documentWindow.handle = windowHandle + "-document";
    documentWindow.parentWindowHandle = windowHandle;
    documentWindow.type = ElementType::Window;
    documentWindow.name = "Document Window";
    documentWindow.isVisible = true;
    documentWindow.isEnabled = true;
    documentWindow.isFocusable = true;
    documentWindow.x = 0;
    documentWindow.y = 0;
    documentWindow.width = 1200;
    documentWindow.height = 800;
    documentWindow.discoveredAt = std::chrono::steady_clock::now();
    elements.push_back(documentWindow);

    // Document text area
    UIElement textArea;
    textArea.handle = windowHandle + "-textarea";
    textArea.parentWindowHandle = windowHandle;
    textArea.parentElementHandle = documentWindow.handle;
    textArea.type = ElementType::TextField;
    textArea.name = "Document Text";
    textArea.value = "This is a sample document text content...";
    textArea.isVisible = true;
    textArea.isEnabled = true;
    textArea.isFocusable = true;
    textArea.isClickable = true;
    textArea.x = 50;
    textArea.y = 120;
    textArea.width = 700;
    textArea.height = 500;
    textArea.accessibilityLabel = "Document content";
    textArea.discoveredAt = std::chrono::steady_clock::now();
    elements.push_back(textArea);

    // Save button
    UIElement saveButton;
    saveButton.handle = windowHandle + "-save";
    saveButton.parentWindowHandle = windowHandle;
    saveButton.type = ElementType::Button;
    saveButton.name = "Save";
    saveButton.isVisible = true;
    saveButton.isEnabled = true;
    saveButton.isFocusable = true;
    saveButton.isClickable = true;
    saveButton.x = 50;
    saveButton.y = 50;
    saveButton.width = 60;
    saveButton.height = 30;
    saveButton.accessibilityLabel = "Save document";
    saveButton.discoveredAt = std::chrono::steady_clock::now();
    elements.push_back(saveButton);

    // Print button
    UIElement printButton;
    printButton.handle = windowHandle + "-print";
    printButton.parentWindowHandle = windowHandle;
    printButton.type = ElementType::Button;
    printButton.name = "Print";
    printButton.isVisible = true;
    printButton.isEnabled = true;
    printButton.isFocusable = true;
    printButton.isClickable = true;
    printButton.x = 120;
    printButton.y = 50;
    printButton.width = 60;
    printButton.height = 30;
    printButton.accessibilityLabel = "Print document";
    printButton.discoveredAt = std::chrono::steady_clock::now();
    elements.push_back(printButton);

    // Font size combo box
    UIElement fontSizeCombo;
    fontSizeCombo.handle = windowHandle + "-fontsize";
    fontSizeCombo.parentWindowHandle = windowHandle;
    fontSizeCombo.type = ElementType::ComboBox;
    fontSizeCombo.name = "Font Size";
    fontSizeCombo.value = "12";
    fontSizeCombo.isVisible = true;
    fontSizeCombo.isEnabled = true;
    fontSizeCombo.isFocusable = true;
    fontSizeCombo.isClickable = true;
    fontSizeCombo.x = 300;
    fontSizeCombo.y = 50;
    fontSizeCombo.width = 60;
    fontSizeCombo.height = 30;
    fontSizeCombo.accessibilityLabel = "Font size";
    fontSizeCombo.discoveredAt = std::chrono::steady_clock::now();
    elements.push_back(fontSizeCombo);

    // Bold button
    UIElement boldButton;
    boldButton.handle = windowHandle + "-bold";
    boldButton.parentWindowHandle = windowHandle;
    boldButton.type = ElementType::Button;
    boldButton.name = "Bold";
    boldButton.isVisible = true;
    boldButton.isEnabled = true;
    boldButton.isFocusable = true;
    boldButton.isClickable = true;
    boldButton.x = 400;
    boldButton.y = 50;
    boldButton.width = 30;
    boldButton.height = 30;
    boldButton.accessibilityLabel = "Bold formatting";
    boldButton.discoveredAt = std::chrono::steady_clock::now();
    elements.push_back(boldButton);

    // Italic button
    UIElement italicButton;
    italicButton.handle = windowHandle + "-italic";
    italicButton.parentWindowHandle = windowHandle;
    italicButton.type = ElementType::Button;
    italicButton.name = "Italic";
    italicButton.isVisible = true;
    italicButton.isEnabled = true;
    italicButton.isFocusable = true;
    italicButton.isClickable = true;
    italicButton.x = 440;
    italicButton.y = 50;
    italicButton.width = 30;
    italicButton.height = 30;
    italicButton.accessibilityLabel = "Italic formatting";
    italicButton.discoveredAt = std::chrono::steady_clock::now();
    elements.push_back(italicButton);

    // Search/Find textbox
    UIElement findBox;
    findBox.handle = windowHandle + "-find";
    findBox.parentWindowHandle = windowHandle;
    findBox.type = ElementType::TextField;
    findBox.name = "Find";
    findBox.value = "";
    findBox.isVisible = true;
    findBox.isEnabled = true;
    findBox.isFocusable = true;
    findBox.isClickable = true;
    findBox.x = 800;
    findBox.y = 50;
    findBox.width = 150;
    findBox.height = 30;
    findBox.accessibilityLabel = "Find text";
    findBox.discoveredAt = std::chrono::steady_clock::now();
    elements.push_back(findBox);
}

void CocoaElementEnumerator::createBrowserElements(std::vector<UIElement>& elements, const std::string& windowHandle, const std::string& /* windowTitle */) {
    // Reuse the original browser implementation but as a separate method

    // Create representative elements for a Chrome browser window
    UIElement browserWindow;
    browserWindow.handle = windowHandle + "-window";
    browserWindow.parentWindowHandle = windowHandle;
    browserWindow.type = ElementType::Window;
    browserWindow.name = "Browser Window";
    browserWindow.isVisible = true;
    browserWindow.isEnabled = true;
    browserWindow.isFocusable = true;
    browserWindow.x = 0;
    browserWindow.y = 0;
    browserWindow.width = 1200;
    browserWindow.height = 800;
    browserWindow.discoveredAt = std::chrono::steady_clock::now();
    elements.push_back(browserWindow);

    // Address bar (URL text field)
    UIElement addressBar;
    addressBar.handle = windowHandle + "-addressbar";
    addressBar.parentWindowHandle = windowHandle;
    addressBar.parentElementHandle = browserWindow.handle;
    addressBar.type = ElementType::TextField;
    addressBar.name = "Address Bar";
    addressBar.value = "https://github.com/freedomw1987/window-manager";
    addressBar.isVisible = true;
    addressBar.isEnabled = true;
    addressBar.isFocusable = true;
    addressBar.isClickable = true;
    addressBar.x = 100;
    addressBar.y = 50;
    addressBar.width = 800;
    addressBar.height = 32;
    addressBar.accessibilityLabel = "Address and search bar";
    addressBar.discoveredAt = std::chrono::steady_clock::now();
    elements.push_back(addressBar);

    // Back button
    UIElement backButton;
    backButton.handle = windowHandle + "-back";
    backButton.parentWindowHandle = windowHandle;
    backButton.parentElementHandle = browserWindow.handle;
    backButton.type = ElementType::Button;
    backButton.name = "Back";
    backButton.isVisible = true;
    backButton.isEnabled = true;
    backButton.isFocusable = true;
    backButton.isClickable = true;
    backButton.x = 20;
    backButton.y = 50;
    backButton.width = 32;
    backButton.height = 32;
    backButton.accessibilityLabel = "Go back";
    backButton.discoveredAt = std::chrono::steady_clock::now();
    elements.push_back(backButton);

    // Reload button
    UIElement reloadButton;
    reloadButton.handle = windowHandle + "-reload";
    reloadButton.parentWindowHandle = windowHandle;
    reloadButton.parentElementHandle = browserWindow.handle;
    reloadButton.type = ElementType::Button;
    reloadButton.name = "Reload";
    reloadButton.isVisible = true;
    reloadButton.isEnabled = true;
    reloadButton.isFocusable = true;
    reloadButton.isClickable = true;
    reloadButton.x = 920;
    reloadButton.y = 50;
    reloadButton.width = 32;
    reloadButton.height = 32;
    reloadButton.accessibilityLabel = "Reload this page";
    reloadButton.discoveredAt = std::chrono::steady_clock::now();
    elements.push_back(reloadButton);
}

void CocoaElementEnumerator::createTerminalElements(std::vector<UIElement>& elements, const std::string& windowHandle, const std::string& /* windowTitle */) {
    // Create typical terminal elements

    // Terminal window
    UIElement terminalWindow;
    terminalWindow.handle = windowHandle + "-terminal";
    terminalWindow.parentWindowHandle = windowHandle;
    terminalWindow.type = ElementType::Window;
    terminalWindow.name = "Terminal Window";
    terminalWindow.isVisible = true;
    terminalWindow.isEnabled = true;
    terminalWindow.isFocusable = true;
    terminalWindow.x = 0;
    terminalWindow.y = 0;
    terminalWindow.width = 800;
    terminalWindow.height = 600;
    terminalWindow.discoveredAt = std::chrono::steady_clock::now();
    elements.push_back(terminalWindow);

    // Terminal text area
    UIElement terminalText;
    terminalText.handle = windowHandle + "-text";
    terminalText.parentWindowHandle = windowHandle;
    terminalText.parentElementHandle = terminalWindow.handle;
    terminalText.type = ElementType::TextField;
    terminalText.name = "Terminal";
    terminalText.value = "$ window-manager list --window a4eb";
    terminalText.isVisible = true;
    terminalText.isEnabled = true;
    terminalText.isFocusable = true;
    terminalText.isClickable = true;
    terminalText.x = 10;
    terminalText.y = 30;
    terminalText.width = 780;
    terminalText.height = 550;
    terminalText.accessibilityLabel = "Terminal output";
    terminalText.discoveredAt = std::chrono::steady_clock::now();
    elements.push_back(terminalText);

    // Command prompt
    UIElement prompt;
    prompt.handle = windowHandle + "-prompt";
    prompt.parentWindowHandle = windowHandle;
    prompt.type = ElementType::Label;
    prompt.name = "Command Prompt";
    prompt.value = "$ ";
    prompt.isVisible = true;
    prompt.isEnabled = true;
    prompt.isFocusable = false;
    prompt.isClickable = false;
    prompt.x = 10;
    prompt.y = 580;
    prompt.width = 20;
    prompt.height = 15;
    prompt.discoveredAt = std::chrono::steady_clock::now();
    elements.push_back(prompt);
}

void CocoaElementEnumerator::createGenericElements(std::vector<UIElement>& elements, const std::string& windowHandle, const std::string& /* windowTitle */) {
    // Create generic application elements

    // Main window
    UIElement mainWindow;
    mainWindow.handle = windowHandle + "-window";
    mainWindow.parentWindowHandle = windowHandle;
    mainWindow.type = ElementType::Window;
    mainWindow.name = "Application Window";
    mainWindow.isVisible = true;
    mainWindow.isEnabled = true;
    mainWindow.isFocusable = true;
    mainWindow.x = 0;
    mainWindow.y = 0;
    mainWindow.width = 800;
    mainWindow.height = 600;
    mainWindow.discoveredAt = std::chrono::steady_clock::now();
    elements.push_back(mainWindow);

    // Generic OK button
    UIElement okButton;
    okButton.handle = windowHandle + "-ok";
    okButton.parentWindowHandle = windowHandle;
    okButton.type = ElementType::Button;
    okButton.name = "OK";
    okButton.isVisible = true;
    okButton.isEnabled = true;
    okButton.isFocusable = true;
    okButton.isClickable = true;
    okButton.x = 350;
    okButton.y = 500;
    okButton.width = 80;
    okButton.height = 30;
    okButton.discoveredAt = std::chrono::steady_clock::now();
    elements.push_back(okButton);

    // Generic Cancel button
    UIElement cancelButton;
    cancelButton.handle = windowHandle + "-cancel";
    cancelButton.parentWindowHandle = windowHandle;
    cancelButton.type = ElementType::Button;
    cancelButton.name = "Cancel";
    cancelButton.isVisible = true;
    cancelButton.isEnabled = true;
    cancelButton.isFocusable = true;
    cancelButton.isClickable = true;
    cancelButton.x = 450;
    cancelButton.y = 500;
    cancelButton.width = 80;
    cancelButton.height = 30;
    cancelButton.discoveredAt = std::chrono::steady_clock::now();
    elements.push_back(cancelButton);
}

// Real Accessibility API implementation
AXUIElementRef CocoaElementEnumerator::getWindowElement(const std::string& windowHandle) {
    CGWindowID windowId = stringToHandle(windowHandle);
    if (windowId == 0) {
        return nullptr;
    }

    // Get the process ID for this window
    CFArrayRef windowArray = CGWindowListCopyWindowInfo(kCGWindowListOptionAll, kCGNullWindowID);
    if (!windowArray) {
        return nullptr;
    }

    pid_t windowPid = 0;
    CFIndex windowCount = CFArrayGetCount(windowArray);
    for (CFIndex i = 0; i < windowCount; i++) {
        CFDictionaryRef windowInfo = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(windowArray, i));
        CFNumberRef windowIdRef = static_cast<CFNumberRef>(CFDictionaryGetValue(windowInfo, kCGWindowNumber));

        if (windowIdRef) {
            CGWindowID currentWindowId;
            CFNumberGetValue(windowIdRef, kCGWindowIDCFNumberType, &currentWindowId);

            if (currentWindowId == windowId) {
                CFNumberRef pidRef = static_cast<CFNumberRef>(CFDictionaryGetValue(windowInfo, kCGWindowOwnerPID));
                if (pidRef) {
                    CFNumberGetValue(pidRef, kCFNumberIntType, &windowPid);
                }
                break;
            }
        }
    }
    CFRelease(windowArray);

    if (windowPid == 0) {
        return nullptr;
    }

    // Create AXUIElement for the application
    AXUIElementRef appElement = AXUIElementCreateApplication(windowPid);
    if (!appElement) {
        return nullptr;
    }

    // Get all windows for this application
    CFArrayRef windows = nullptr;
    AXError error = AXUIElementCopyAttributeValues(appElement, kAXWindowsAttribute, 0, 1000, &windows);
    if (error != kAXErrorSuccess || !windows) {
        CFRelease(appElement);
        return nullptr;
    }

    // Since there's no direct way to match AX window with CGWindowID,
    // we'll use the first available window if there's only one,
    // or try to match by title if available
    AXUIElementRef targetWindow = nullptr;
    CFIndex windowsCount = CFArrayGetCount(windows);

    if (windowsCount == 1) {
        // If there's only one window, use it
        targetWindow = static_cast<AXUIElementRef>(CFArrayGetValueAtIndex(windows, 0));
        CFRetain(targetWindow);
    } else if (windowsCount > 1) {
        // Try to find the main window or the first visible window
        for (CFIndex i = 0; i < windowsCount; i++) {
            AXUIElementRef window = static_cast<AXUIElementRef>(CFArrayGetValueAtIndex(windows, i));

            // Check if window is the main window
            CFTypeRef isMainValue = nullptr;
            error = AXUIElementCopyAttributeValue(window, kAXMainAttribute, &isMainValue);
            if (error == kAXErrorSuccess && isMainValue) {
                Boolean isMain = false;
                if (CFGetTypeID(isMainValue) == CFBooleanGetTypeID()) {
                    isMain = CFBooleanGetValue(static_cast<CFBooleanRef>(isMainValue));
                }
                CFRelease(isMainValue);

                if (isMain) {
                    targetWindow = window;
                    CFRetain(targetWindow);
                    break;
                }
            }
        }

        // If no main window found, use the first window
        if (!targetWindow && windowsCount > 0) {
            targetWindow = static_cast<AXUIElementRef>(CFArrayGetValueAtIndex(windows, 0));
            CFRetain(targetWindow);
        }
    }

    CFRelease(windows);
    CFRelease(appElement);
    return targetWindow;
}

std::vector<UIElement> CocoaElementEnumerator::traverseElementTree(AXUIElementRef rootElement, const std::string& windowHandle) {
    std::vector<UIElement> elements;

    if (!rootElement) {
        return elements;
    }

    // Add the root element (window) itself
    UIElement windowElement = createUIElementFromAXElement(rootElement, windowHandle);
    if (!windowElement.handle.empty()) {
        elements.push_back(windowElement);
    }

    // Recursively traverse child elements
    std::vector<AXUIElementRef> children = getElementChildren(rootElement);
    for (AXUIElementRef child : children) {
        traverseElementTreeRecursive(child, windowHandle, elements, 0, 10); // Max depth 10
    }

    return elements;
}

void CocoaElementEnumerator::traverseElementTreeRecursive(AXUIElementRef element, const std::string& windowHandle, std::vector<UIElement>& elements, int depth, int maxDepth) {
    if (depth > maxDepth || !element) {
        return;
    }

    // Create UI element from AX element
    UIElement uiElement = createUIElementFromAXElement(element, windowHandle);
    if (!uiElement.handle.empty()) {
        elements.push_back(uiElement);
    }

    // Recursively process children
    std::vector<AXUIElementRef> children = getElementChildren(element);
    for (AXUIElementRef child : children) {
        traverseElementTreeRecursive(child, windowHandle, elements, depth + 1, maxDepth);
    }
}

UIElement CocoaElementEnumerator::createUIElementFromAXElement(AXUIElementRef element, const std::string& parentWindowHandle) {
    UIElement uiElement;

    if (!element) {
        return uiElement;
    }

    // Generate unique handle for this element
    uiElement.handle = parentWindowHandle + "-" + std::to_string(reinterpret_cast<uintptr_t>(element));
    uiElement.parentWindowHandle = parentWindowHandle;

    // Get element role (type)
    CFStringRef role = getStringAttribute(element, kAXRoleAttribute);
    if (role) {
        std::string roleStr = getElementRole(element);
        uiElement.type = getElementTypeFromRole(roleStr);
    }

    // Get element title/name
    uiElement.name = getElementName(element);

    // Get element value
    CFStringRef value = getStringAttribute(element, kAXValueAttribute);
    if (value) {
        char valueBuffer[1024];
        CFStringGetCString(value, valueBuffer, sizeof(valueBuffer), kCFStringEncodingUTF8);
        uiElement.value = valueBuffer;
        CFRelease(value);
    }

    // Get element position and size
    CGRect bounds = getElementBounds(element);
    uiElement.x = static_cast<int>(bounds.origin.x);
    uiElement.y = static_cast<int>(bounds.origin.y);
    uiElement.width = static_cast<int>(bounds.size.width);
    uiElement.height = static_cast<int>(bounds.size.height);

    // Get element state
    uiElement.isVisible = isElementVisible(element);
    uiElement.isEnabled = getBoolAttribute(element, kAXEnabledAttribute);
    uiElement.isFocusable = getBoolAttribute(element, kAXFocusedAttribute);

    // Get accessibility description
    CFStringRef description = getStringAttribute(element, kAXDescriptionAttribute);
    if (description) {
        char descBuffer[512];
        CFStringGetCString(description, descBuffer, sizeof(descBuffer), kCFStringEncodingUTF8);
        uiElement.accessibilityLabel = descBuffer;
        CFRelease(description);
    }

    uiElement.discoveredAt = std::chrono::steady_clock::now();

    return uiElement;
}

ElementType CocoaElementEnumerator::getElementTypeFromRole(const std::string& role) {
    if (role == "AXButton") return ElementType::Button;
    if (role == "AXTextField") return ElementType::TextField;
    if (role == "AXStaticText") return ElementType::Label;
    if (role == "AXWindow") return ElementType::Window;
    if (role == "AXComboBox") return ElementType::ComboBox;
    if (role == "AXCheckBox") return ElementType::CheckBox;
    if (role == "AXRadioButton") return ElementType::RadioButton;
    if (role == "AXTable") return ElementType::Table;
    if (role == "AXImage") return ElementType::Image;
    if (role == "AXLink") return ElementType::Link;
    if (role == "AXMenu") return ElementType::Menu;
    if (role == "AXMenuItem") return ElementType::MenuItem;
    if (role == "AXScrollBar") return ElementType::ScrollBar;
    if (role == "AXSlider") return ElementType::Slider;
    if (role == "AXProgressIndicator") return ElementType::ProgressBar;
    if (role == "AXGroup" || role == "AXSplitGroup") return ElementType::Pane;
    return ElementType::Unknown;
}

// Accessibility API helper implementations
CFStringRef CocoaElementEnumerator::getStringAttribute(AXUIElementRef element, CFStringRef attribute) {
    CFTypeRef value = nullptr;
    AXError error = AXUIElementCopyAttributeValue(element, attribute, &value);
    if (error == kAXErrorSuccess && value && CFGetTypeID(value) == CFStringGetTypeID()) {
        return static_cast<CFStringRef>(value);
    }
    if (value) CFRelease(value);
    return nullptr;
}

bool CocoaElementEnumerator::getBoolAttribute(AXUIElementRef element, CFStringRef attribute) {
    CFTypeRef value = nullptr;
    AXError error = AXUIElementCopyAttributeValue(element, attribute, &value);
    bool result = false;
    if (error == kAXErrorSuccess && value) {
        if (CFGetTypeID(value) == CFBooleanGetTypeID()) {
            result = CFBooleanGetValue(static_cast<CFBooleanRef>(value));
        }
        CFRelease(value);
    }
    return result;
}

CGRect CocoaElementEnumerator::getElementBounds(AXUIElementRef element) {
    CGRect bounds = CGRectZero;

    // Get position
    CFTypeRef positionValue = nullptr;
    AXError error = AXUIElementCopyAttributeValue(element, kAXPositionAttribute, &positionValue);
    if (error == kAXErrorSuccess && positionValue) {
        CGPoint position;
        if (AXValueGetValue(static_cast<AXValueRef>(positionValue), kAXValueTypeCGPoint, &position)) {
            bounds.origin = position;
        }
        CFRelease(positionValue);
    }

    // Get size
    CFTypeRef sizeValue = nullptr;
    error = AXUIElementCopyAttributeValue(element, kAXSizeAttribute, &sizeValue);
    if (error == kAXErrorSuccess && sizeValue) {
        CGSize size;
        if (AXValueGetValue(static_cast<AXValueRef>(sizeValue), kAXValueTypeCGSize, &size)) {
            bounds.size = size;
        }
        CFRelease(sizeValue);
    }

    return bounds;
}

bool CocoaElementEnumerator::isElementVisible(AXUIElementRef element) {
    // Check if element is visible
    CFTypeRef value = nullptr;
    AXError error = AXUIElementCopyAttributeValue(element, kAXHiddenAttribute, &value);
    if (error == kAXErrorSuccess && value) {
        bool isHidden = false;
        if (CFGetTypeID(value) == CFBooleanGetTypeID()) {
            isHidden = CFBooleanGetValue(static_cast<CFBooleanRef>(value));
        }
        CFRelease(value);
        return !isHidden;
    }
    return true; // Assume visible if we can't determine
}

std::string CocoaElementEnumerator::getElementName(AXUIElementRef element) {
    std::string name;

    // Try title first
    CFStringRef title = getStringAttribute(element, kAXTitleAttribute);
    if (title) {
        char buffer[512];
        CFStringGetCString(title, buffer, sizeof(buffer), kCFStringEncodingUTF8);
        name = buffer;
        CFRelease(title);
        return name;
    }

    // Try label
    CFStringRef label = getStringAttribute(element, kAXTitleAttribute);
    if (label) {
        char buffer[512];
        CFStringGetCString(label, buffer, sizeof(buffer), kCFStringEncodingUTF8);
        name = buffer;
        CFRelease(label);
        return name;
    }

    // Try identifier
    CFStringRef identifier = getStringAttribute(element, kAXIdentifierAttribute);
    if (identifier) {
        char buffer[512];
        CFStringGetCString(identifier, buffer, sizeof(buffer), kCFStringEncodingUTF8);
        name = buffer;
        CFRelease(identifier);
        return name;
    }

    return "Unnamed Element";
}

std::string CocoaElementEnumerator::getElementRole(AXUIElementRef element) {
    CFStringRef role = getStringAttribute(element, kAXRoleAttribute);
    if (role) {
        char buffer[256];
        CFStringGetCString(role, buffer, sizeof(buffer), kCFStringEncodingUTF8);
        std::string roleStr = buffer;
        CFRelease(role);
        return roleStr;
    }
    return "AXUnknown";
}

std::vector<AXUIElementRef> CocoaElementEnumerator::getElementChildren(AXUIElementRef element) {
    std::vector<AXUIElementRef> children;

    CFArrayRef childrenArray = nullptr;
    AXError error = AXUIElementCopyAttributeValue(element, kAXChildrenAttribute, reinterpret_cast<CFTypeRef*>(&childrenArray));

    if (error == kAXErrorSuccess && childrenArray) {
        CFIndex count = CFArrayGetCount(childrenArray);
        for (CFIndex i = 0; i < count; i++) {
            AXUIElementRef child = static_cast<AXUIElementRef>(CFArrayGetValueAtIndex(childrenArray, i));
            if (child) {
                children.push_back(child);
            }
        }
        CFRelease(childrenArray);
    }

    return children;
}

} // namespace WindowManager

#endif // MACOS_PLATFORM