#include "focus_request.hpp"
#include <sstream>
#include <random>

namespace WindowManager {

// Default constructor
FocusRequest::FocusRequest()
    : timestamp(std::chrono::steady_clock::now())
    , requestId(generateRequestId())
    , crossWorkspace(false) {
}

// Parameterized constructor
FocusRequest::FocusRequest(const std::string& handle, const std::string& id)
    : targetHandle(handle)
    , timestamp(std::chrono::steady_clock::now())
    , requestId(id.empty() ? generateRequestId() : id)
    , crossWorkspace(false) {
}

// Full constructor with workspace information
FocusRequest::FocusRequest(const std::string& handle, const std::string& id,
                         bool requiresWorkspaceSwitch, const std::string& sourceWS,
                         const std::string& targetWS)
    : targetHandle(handle)
    , timestamp(std::chrono::steady_clock::now())
    , requestId(id.empty() ? generateRequestId() : id)
    , crossWorkspace(requiresWorkspaceSwitch)
    , sourceWorkspace(sourceWS)
    , targetWorkspace(targetWS) {
}

// Validation methods
bool FocusRequest::isValid() const {
    return !targetHandle.empty() && !requestId.empty();
}

bool FocusRequest::hasValidHandle() const {
    if (targetHandle.empty()) {
        return false;
    }

    // Basic handle format validation
    for (char c : targetHandle) {
        if (!std::isxdigit(c)) {
            return false;
        }
    }

    return targetHandle.length() <= 16; // Max 64-bit handle length
}

bool FocusRequest::requiresWorkspaceSwitch() const {
    return crossWorkspace;
}

// Utility methods
std::string FocusRequest::toString() const {
    std::ostringstream oss;

    oss << "FocusRequest {" << std::endl;
    oss << "  Handle: " << targetHandle << std::endl;
    oss << "  RequestId: " << requestId << std::endl;
    oss << "  CrossWorkspace: " << (crossWorkspace ? "Yes" : "No") << std::endl;

    if (!sourceWorkspace.empty()) {
        oss << "  SourceWorkspace: " << sourceWorkspace << std::endl;
    }

    if (!targetWorkspace.empty()) {
        oss << "  TargetWorkspace: " << targetWorkspace << std::endl;
    }

    oss << "}";
    return oss.str();
}

std::string FocusRequest::toJson() const {
    std::ostringstream oss;

    oss << "{\n";
    oss << "  \"targetHandle\": \"" << targetHandle << "\",\n";
    oss << "  \"requestId\": \"" << requestId << "\",\n";
    oss << "  \"crossWorkspace\": " << (crossWorkspace ? "true" : "false");

    if (!sourceWorkspace.empty()) {
        oss << ",\n  \"sourceWorkspace\": \"" << sourceWorkspace << "\"";
    }

    if (!targetWorkspace.empty()) {
        oss << ",\n  \"targetWorkspace\": \"" << targetWorkspace << "\"";
    }

    oss << "\n}";
    return oss.str();
}

// Generate unique request ID
std::string FocusRequest::generateRequestId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(10000, 99999);

    auto now = std::chrono::steady_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();

    return std::to_string(timestamp) + "_" + std::to_string(dis(gen));
}

// Comparison operators
bool FocusRequest::operator==(const FocusRequest& other) const {
    return requestId == other.requestId;
}

bool FocusRequest::operator!=(const FocusRequest& other) const {
    return !(*this == other);
}

} // namespace WindowManager