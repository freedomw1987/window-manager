#include "focus_operation.hpp"
#include <sstream>
#include <iomanip>

namespace WindowManager {

// Default constructor
FocusOperation::FocusOperation()
    : status(FocusStatus::PENDING)
    , startTime(std::chrono::steady_clock::now())
    , workspaceSwitched(false)
    , windowRestored(false)
    , platformErrorCode(0) {
}

// Constructor from focus request
FocusOperation::FocusOperation(const FocusRequest& req)
    : request(req)
    , status(FocusStatus::PENDING)
    , startTime(std::chrono::steady_clock::now())
    , workspaceSwitched(false)
    , windowRestored(false)
    , platformErrorCode(0) {
}

// Constructor with initial status
FocusOperation::FocusOperation(const FocusRequest& req, FocusStatus initialStatus)
    : request(req)
    , status(initialStatus)
    , startTime(std::chrono::steady_clock::now())
    , workspaceSwitched(false)
    , windowRestored(false)
    , platformErrorCode(0) {
}

// Status update methods
void FocusOperation::setStatus(FocusStatus newStatus) {
    status = newStatus;
}

void FocusOperation::complete() {
    status = FocusStatus::COMPLETED;
    endTime = std::chrono::steady_clock::now();
}

void FocusOperation::fail(const std::string& error, int errorCode) {
    status = FocusStatus::FAILED;
    endTime = std::chrono::steady_clock::now();
    errorMessage = error;
    platformErrorCode = errorCode;
}

void FocusOperation::markWorkspaceSwitched() {
    workspaceSwitched = true;
}

void FocusOperation::markWindowRestored() {
    windowRestored = true;
}

// Timing methods
std::chrono::milliseconds FocusOperation::getDuration() const {
    auto end = endTime.value_or(std::chrono::steady_clock::now());
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - startTime);
}

bool FocusOperation::isComplete() const {
    return status == FocusStatus::COMPLETED || status == FocusStatus::FAILED || status == FocusStatus::CANCELLED;
}

bool FocusOperation::isSuccessful() const {
    return status == FocusStatus::COMPLETED;
}

bool FocusOperation::hasFailed() const {
    return status == FocusStatus::FAILED;
}

// Validation methods
bool FocusOperation::isValid() const {
    return !request.targetHandle.empty() && request.isValid();
}

bool FocusOperation::meetsPerformanceRequirements() const {
    auto duration = getDuration();

    // Check performance requirements based on operation type
    if (workspaceSwitched) {
        // Cross-workspace focus: < 2 seconds (SC-002)
        return duration <= std::chrono::milliseconds(2000);
    } else {
        // Current workspace focus: < 1 second (SC-001)
        return duration <= std::chrono::milliseconds(1000);
    }
}

// Output methods
std::string FocusOperation::toString() const {
    std::ostringstream oss;

    oss << "FocusOperation {" << std::endl;
    oss << "  Handle: " << request.targetHandle << std::endl;
    oss << "  Status: " << getStatusString() << std::endl;
    oss << "  Duration: " << getDuration().count() << "ms" << std::endl;

    if (workspaceSwitched) {
        oss << "  Workspace Switched: Yes" << std::endl;
    }

    if (windowRestored) {
        oss << "  Window Restored: Yes" << std::endl;
    }

    if (!errorMessage.empty()) {
        oss << "  Error: " << errorMessage << std::endl;
    }

    oss << "}";
    return oss.str();
}

std::string FocusOperation::toJson() const {
    std::ostringstream oss;

    oss << "{\n";
    oss << "  \"request\": {\n";
    oss << "    \"handle\": \"" << request.targetHandle << "\",\n";
    oss << "    \"requestId\": \"" << request.requestId << "\",\n";
    oss << "    \"crossWorkspace\": " << (request.crossWorkspace ? "true" : "false") << "\n";
    oss << "  },\n";
    oss << "  \"status\": \"" << getStatusString() << "\",\n";
    oss << "  \"duration_ms\": " << getDuration().count() << ",\n";
    oss << "  \"workspaceSwitched\": " << (workspaceSwitched ? "true" : "false") << ",\n";
    oss << "  \"windowRestored\": " << (windowRestored ? "true" : "false");

    if (!errorMessage.empty()) {
        oss << ",\n  \"error\": \"" << errorMessage << "\"";
    }

    if (platformErrorCode != 0) {
        oss << ",\n  \"platformErrorCode\": " << platformErrorCode;
    }

    oss << "\n}";
    return oss.str();
}

std::string FocusOperation::getStatusString() const {
    switch (status) {
        case FocusStatus::PENDING:
            return "PENDING";
        case FocusStatus::VALIDATING:
            return "VALIDATING";
        case FocusStatus::SWITCHING_WORKSPACE:
            return "SWITCHING_WORKSPACE";
        case FocusStatus::FOCUSING:
            return "FOCUSING";
        case FocusStatus::RESTORING:
            return "RESTORING";
        case FocusStatus::COMPLETED:
            return "COMPLETED";
        case FocusStatus::FAILED:
            return "FAILED";
        case FocusStatus::CANCELLED:
            return "CANCELLED";
        default:
            return "UNKNOWN";
    }
}

// Comparison operators
bool FocusOperation::operator==(const FocusOperation& other) const {
    return request.requestId == other.request.requestId;
}

bool FocusOperation::operator!=(const FocusOperation& other) const {
    return !(*this == other);
}

} // namespace WindowManager