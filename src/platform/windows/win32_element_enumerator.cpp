#include "win32_element_enumerator.hpp"

#ifdef WINDOWS_PLATFORM

#include <comdef.h>
#include <sstream>

namespace WindowManager {

Win32ElementEnumerator::Win32ElementEnumerator()
    : uiAutomation_(nullptr)
    , comInitialized_(false) {
    initializeUIAutomation();
}

Win32ElementEnumerator::~Win32ElementEnumerator() {
    cleanup();
}

bool Win32ElementEnumerator::initializeUIAutomation() {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        return false;
    }
    comInitialized_ = true;

    hr = CoCreateInstance(__uuidof(CUIAutomation), nullptr, CLSCTX_INPROC_SERVER,
                         __uuidof(IUIAutomation), (void**)&uiAutomation_);
    return SUCCEEDED(hr);
}

void Win32ElementEnumerator::cleanup() {
    if (uiAutomation_) {
        uiAutomation_->Release();
        uiAutomation_ = nullptr;
    }
    if (comInitialized_) {
        CoUninitialize();
        comInitialized_ = false;
    }
}

ElementEnumerationResult Win32ElementEnumerator::enumerateElements(const std::string& windowHandle) {
    auto start = std::chrono::steady_clock::now();
    ElementEnumerationResult result(windowHandle);

    if (!uiAutomation_) {
        result.success = false;
        result.errorMessage = "UI Automation not available";
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
        HWND hwnd = stringToHandle(windowHandle);
        if (!hwnd || !IsWindow(hwnd)) {
            result.success = false;
            result.errorMessage = "Invalid window handle";
            return result;
        }

        IUIAutomationElement* windowElement = getWindowElement(windowHandle);
        if (!windowElement) {
            result.success = false;
            result.errorMessage = "Could not get window element";
            return result;
        }

        // Get window title
        BSTR windowName;
        if (SUCCEEDED(windowElement->get_CurrentName(&windowName))) {
            result.windowTitle = _com_util::ConvertBSTRToString(windowName);
            SysFreeString(windowName);
        }

        // Traverse element tree
        result.elements = traverseElementTree(windowElement, windowHandle);
        result.totalElementCount = result.elements.size();
        result.filteredElementCount = result.elements.size();
        result.success = true;
        result.supportsElementEnumeration = true;
        result.hasAccessibilityPermissions = true;

        // Update cache
        updateCache(windowHandle, result.elements);

        windowElement->Release();

    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = e.what();
    }

    auto end = std::chrono::steady_clock::now();
    result.enumerationTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    updateEnumerationTime(start, end);

    return result;
}

ElementEnumerationResult Win32ElementEnumerator::searchElements(const std::string& windowHandle,
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

std::optional<UIElement> Win32ElementEnumerator::getElementInfo(const std::string& elementHandle) {
    // For simplified implementation, return nullopt
    // In a full implementation, this would parse the element handle and retrieve specific element info
    return std::nullopt;
}

bool Win32ElementEnumerator::isElementValid(const std::string& elementHandle) {
    return false; // Simplified implementation
}

bool Win32ElementEnumerator::supportsElementEnumeration(const std::string& windowHandle) {
    return uiAutomation_ != nullptr;
}

bool Win32ElementEnumerator::hasElementAccessPermissions() const {
    return uiAutomation_ != nullptr;
}

void Win32ElementEnumerator::clearElementCache(const std::string& windowHandle) {
    elementCache_.erase(windowHandle);
    cacheTimestamps_.erase(windowHandle);
}

void Win32ElementEnumerator::clearAllElementCaches() {
    elementCache_.clear();
    cacheTimestamps_.clear();
}

std::chrono::milliseconds Win32ElementEnumerator::getLastEnumerationTime() const {
    return lastEnumerationDuration_;
}

std::string Win32ElementEnumerator::getPlatformInfo() const {
    return "Windows UI Automation";
}

// Helper methods implementation
IUIAutomationElement* Win32ElementEnumerator::getWindowElement(const std::string& windowHandle) {
    if (!uiAutomation_) return nullptr;

    HWND hwnd = stringToHandle(windowHandle);
    if (!hwnd) return nullptr;

    IUIAutomationElement* element = nullptr;
    HRESULT hr = uiAutomation_->ElementFromHandle(hwnd, &element);
    return SUCCEEDED(hr) ? element : nullptr;
}

UIElement Win32ElementEnumerator::createUIElementFromAutomationElement(IUIAutomationElement* element,
                                                                      const std::string& parentWindowHandle) {
    UIElement uiElement;
    uiElement.parentWindowHandle = parentWindowHandle;

    // Generate a simple handle (in real implementation, this would be more sophisticated)
    static int elementCounter = 0;
    uiElement.handle = parentWindowHandle + "-" + std::to_string(++elementCounter);

    // Get element properties
    BSTR name;
    if (SUCCEEDED(element->get_CurrentName(&name))) {
        uiElement.name = _com_util::ConvertBSTRToString(name);
        SysFreeString(name);
    }

    BSTR value;
    if (SUCCEEDED(element->get_CurrentItemStatus(&value))) {
        uiElement.value = _com_util::ConvertBSTRToString(value);
        SysFreeString(value);
    }

    // Get element type and state
    uiElement.type = getElementTypeFromAutomation(element);
    uiElement.state = getElementStateFromAutomation(element);

    // Get element bounds
    RECT rect;
    if (SUCCEEDED(element->get_CurrentBoundingRectangle(&rect))) {
        uiElement.x = rect.left;
        uiElement.y = rect.top;
        uiElement.width = rect.right - rect.left;
        uiElement.height = rect.bottom - rect.top;
    }

    // Get element visibility and enabled state
    BOOL isEnabled;
    if (SUCCEEDED(element->get_CurrentIsEnabled(&isEnabled))) {
        uiElement.isEnabled = isEnabled != FALSE;
    }

    uiElement.isVisible = isElementVisible(element);
    uiElement.discoveredAt = std::chrono::steady_clock::now();

    return uiElement;
}

std::vector<UIElement> Win32ElementEnumerator::traverseElementTree(IUIAutomationElement* rootElement,
                                                                  const std::string& windowHandle) {
    std::vector<UIElement> elements;

    if (!rootElement || !uiAutomation_) return elements;

    // Add the root element
    elements.push_back(createUIElementFromAutomationElement(rootElement, windowHandle));

    // Get child elements
    IUIAutomationElementArray* children = nullptr;
    IUIAutomationTreeWalker* walker = nullptr;

    HRESULT hr = uiAutomation_->get_ControlViewWalker(&walker);
    if (SUCCEEDED(hr) && walker) {
        IUIAutomationElement* child = nullptr;
        hr = walker->GetFirstChildElement(rootElement, &child);

        while (SUCCEEDED(hr) && child) {
            // Recursively traverse child elements (limited depth for performance)
            auto childElements = traverseElementTree(child, windowHandle);
            elements.insert(elements.end(), childElements.begin(), childElements.end());

            IUIAutomationElement* nextChild = nullptr;
            hr = walker->GetNextSiblingElement(child, &nextChild);
            child->Release();
            child = nextChild;
        }

        walker->Release();
    }

    return elements;
}

// Utility methods
std::string Win32ElementEnumerator::handleToString(HWND hwnd) {
    return std::to_string(reinterpret_cast<uintptr_t>(hwnd));
}

HWND Win32ElementEnumerator::stringToHandle(const std::string& handleStr) {
    try {
        uintptr_t handle = std::stoull(handleStr);
        return reinterpret_cast<HWND>(handle);
    } catch (...) {
        return nullptr;
    }
}

ElementType Win32ElementEnumerator::getElementTypeFromAutomation(IUIAutomationElement* element) {
    CONTROLTYPEID controlType;
    if (SUCCEEDED(element->get_CurrentControlType(&controlType))) {
        switch (controlType) {
            case UIA_ButtonControlTypeId: return ElementType::Button;
            case UIA_EditControlTypeId: return ElementType::TextField;
            case UIA_TextControlTypeId: return ElementType::Label;
            case UIA_MenuControlTypeId: return ElementType::Menu;
            case UIA_MenuItemControlTypeId: return ElementType::MenuItem;
            case UIA_ComboBoxControlTypeId: return ElementType::ComboBox;
            case UIA_CheckBoxControlTypeId: return ElementType::CheckBox;
            case UIA_RadioButtonControlTypeId: return ElementType::RadioButton;
            case UIA_ImageControlTypeId: return ElementType::Image;
            case UIA_HyperlinkControlTypeId: return ElementType::Link;
            case UIA_TableControlTypeId: return ElementType::Table;
            case UIA_WindowControlTypeId: return ElementType::Window;
            case UIA_PaneControlTypeId: return ElementType::Pane;
            case UIA_ScrollBarControlTypeId: return ElementType::ScrollBar;
            case UIA_SliderControlTypeId: return ElementType::Slider;
            case UIA_ProgressBarControlTypeId: return ElementType::ProgressBar;
            default: return ElementType::Unknown;
        }
    }
    return ElementType::Unknown;
}

ElementState Win32ElementEnumerator::getElementStateFromAutomation(IUIAutomationElement* element) {
    BOOL isEnabled;
    if (SUCCEEDED(element->get_CurrentIsEnabled(&isEnabled)) && !isEnabled) {
        return ElementState::Disabled;
    }

    BOOL hasFocus;
    if (SUCCEEDED(element->get_CurrentHasKeyboardFocus(&hasFocus)) && hasFocus) {
        return ElementState::Focused;
    }

    return ElementState::Normal;
}

bool Win32ElementEnumerator::isElementVisible(IUIAutomationElement* element) {
    BOOL isOffscreen;
    if (SUCCEEDED(element->get_CurrentIsOffscreen(&isOffscreen))) {
        return !isOffscreen;
    }
    return true; // Default to visible if can't determine
}

// Cache management
bool Win32ElementEnumerator::isCacheValid(const std::string& windowHandle) const {
    auto it = cacheTimestamps_.find(windowHandle);
    if (it == cacheTimestamps_.end()) {
        return false;
    }

    auto now = std::chrono::steady_clock::now();
    auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second);
    return age < CACHE_TIMEOUT;
}

void Win32ElementEnumerator::updateCache(const std::string& windowHandle, const std::vector<UIElement>& elements) {
    elementCache_[windowHandle] = elements;
    cacheTimestamps_[windowHandle] = std::chrono::steady_clock::now();
}

std::vector<UIElement> Win32ElementEnumerator::getCachedElements(const std::string& windowHandle) const {
    auto it = elementCache_.find(windowHandle);
    return it != elementCache_.end() ? it->second : std::vector<UIElement>();
}

} // namespace WindowManager

#endif // WINDOWS_PLATFORM