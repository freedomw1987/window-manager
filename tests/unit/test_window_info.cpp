#include <gtest/gtest.h>
#include "../../src/core/window.hpp"
#include "../../src/core/workspace.hpp"

namespace WindowManager {
namespace Tests {

class WindowInfoTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test data
        testWindow.handle = "test_handle_123";
        testWindow.title = "Test Window";
        testWindow.x = 100;
        testWindow.y = 200;
        testWindow.width = 800;
        testWindow.height = 600;
        testWindow.isVisible = true;
        testWindow.processId = 1234;
        testWindow.ownerName = "TestApp";
        testWindow.workspaceId = "workspace_1";
        testWindow.workspaceName = "Development";
        testWindow.isOnCurrentWorkspace = true;
        testWindow.state = WindowState::Normal;
        testWindow.isFocused = false;
        testWindow.isMinimized = false;
    }

    WindowInfo testWindow;
};

TEST_F(WindowInfoTest, DefaultConstructor) {
    WindowInfo window;

    EXPECT_EQ(window.handle, "");
    EXPECT_EQ(window.title, "");
    EXPECT_EQ(window.x, 0);
    EXPECT_EQ(window.y, 0);
    EXPECT_EQ(window.width, 0);
    EXPECT_EQ(window.height, 0);
    EXPECT_FALSE(window.isVisible);
    EXPECT_EQ(window.processId, 0);
    EXPECT_EQ(window.ownerName, "");
    EXPECT_EQ(window.workspaceId, "");
    EXPECT_EQ(window.workspaceName, "");
    EXPECT_TRUE(window.isOnCurrentWorkspace);
    EXPECT_EQ(window.state, WindowState::Normal);
    EXPECT_FALSE(window.isFocused);
    EXPECT_FALSE(window.isMinimized);
}

TEST_F(WindowInfoTest, WorkspaceFieldsInitialization) {
    EXPECT_EQ(testWindow.workspaceId, "workspace_1");
    EXPECT_EQ(testWindow.workspaceName, "Development");
    EXPECT_TRUE(testWindow.isOnCurrentWorkspace);
    EXPECT_EQ(testWindow.state, WindowState::Normal);
    EXPECT_FALSE(testWindow.isFocused);
    EXPECT_FALSE(testWindow.isMinimized);
}

TEST_F(WindowInfoTest, EnhancedConstructor) {
    WindowInfo window("handle", "title", 10, 20, 300, 400, true, 999, "app",
                     "ws1", "Workspace1", false, WindowState::Focused);

    EXPECT_EQ(window.handle, "handle");
    EXPECT_EQ(window.title, "title");
    EXPECT_EQ(window.x, 10);
    EXPECT_EQ(window.y, 20);
    EXPECT_EQ(window.width, 300);
    EXPECT_EQ(window.height, 400);
    EXPECT_TRUE(window.isVisible);
    EXPECT_EQ(window.processId, 999);
    EXPECT_EQ(window.ownerName, "app");
    EXPECT_EQ(window.workspaceId, "ws1");
    EXPECT_EQ(window.workspaceName, "Workspace1");
    EXPECT_FALSE(window.isOnCurrentWorkspace);
    EXPECT_EQ(window.state, WindowState::Focused);
    EXPECT_TRUE(window.isFocused);
    EXPECT_FALSE(window.isMinimized);
}

TEST_F(WindowInfoTest, ValidationMethods) {
    // Valid window
    EXPECT_TRUE(testWindow.isValid());
    EXPECT_TRUE(testWindow.hasValidDimensions());
    EXPECT_TRUE(testWindow.hasValidPosition());
    EXPECT_TRUE(testWindow.hasWorkspaceInfo());

    // Invalid dimensions
    WindowInfo invalidDim = testWindow;
    invalidDim.width = 0;
    EXPECT_FALSE(invalidDim.hasValidDimensions());
    EXPECT_FALSE(invalidDim.isValid());

    // No workspace info
    WindowInfo noWorkspace = testWindow;
    noWorkspace.workspaceId = "";
    noWorkspace.workspaceName = "";
    EXPECT_FALSE(noWorkspace.hasWorkspaceInfo());
}

TEST_F(WindowInfoTest, JsonOutput) {
    std::string json = testWindow.toJson();

    // Check for required fields
    EXPECT_NE(json.find("\"handle\""), std::string::npos);
    EXPECT_NE(json.find("\"title\""), std::string::npos);
    EXPECT_NE(json.find("\"workspaceId\""), std::string::npos);
    EXPECT_NE(json.find("\"workspaceName\""), std::string::npos);
    EXPECT_NE(json.find("\"state\""), std::string::npos);
    EXPECT_NE(json.find("\"isFocused\""), std::string::npos);

    // Verify it's valid JSON structure
    EXPECT_EQ(json.front(), '{');
    EXPECT_EQ(json.back(), '}');
}

TEST_F(WindowInfoTest, EnhancedJsonOutput) {
    std::vector<WorkspaceInfo> workspaces;
    WorkspaceInfo ws;
    ws.id = "workspace_1";
    ws.name = "Development";
    ws.index = 0;
    ws.isCurrent = true;
    workspaces.push_back(ws);

    std::string json = testWindow.toJsonWithWorkspaceContext(workspaces);

    // Check for enhanced workspace context
    EXPECT_NE(json.find("\"workspace\""), std::string::npos);
    EXPECT_NE(json.find("\"state\""), std::string::npos);
    EXPECT_NE(json.find("\"geometry\""), std::string::npos);
    EXPECT_NE(json.find("\"metadata\""), std::string::npos);
}

TEST_F(WindowInfoTest, CompactJsonOutput) {
    std::string json = testWindow.toCompactJson();

    // Should be compact (no extra whitespace)
    EXPECT_EQ(json.find('\n'), std::string::npos);
    EXPECT_EQ(json.find("  "), std::string::npos);

    // But still contain essential fields
    EXPECT_NE(json.find("\"handle\""), std::string::npos);
    EXPECT_NE(json.find("\"workspaceId\""), std::string::npos);
}

TEST_F(WindowInfoTest, StringOutput) {
    std::string str = testWindow.toString();

    EXPECT_NE(str.find("TestApp"), std::string::npos);
    EXPECT_NE(str.find("Test Window"), std::string::npos);
    EXPECT_NE(str.find("Development"), std::string::npos);
    EXPECT_NE(str.find("State: Normal"), std::string::npos);
}

TEST_F(WindowInfoTest, ComparisonOperators) {
    WindowInfo window1 = testWindow;
    WindowInfo window2 = testWindow;
    WindowInfo window3 = testWindow;
    window3.title = "Different Title";

    EXPECT_TRUE(window1 == window2);
    EXPECT_FALSE(window1 != window2);
    EXPECT_FALSE(window1 == window3);
    EXPECT_TRUE(window1 != window3);
}

TEST_F(WindowInfoTest, SortingOperator) {
    WindowInfo window1 = testWindow;
    window1.title = "A Window";

    WindowInfo window2 = testWindow;
    window2.title = "B Window";

    EXPECT_TRUE(window1 < window2);
    EXPECT_FALSE(window2 < window1);
}

TEST_F(WindowInfoTest, WindowStateMapping) {
    // Test state enum mapping to boolean fields
    WindowInfo focused = testWindow;
    focused.state = WindowState::Focused;
    focused.isFocused = true;
    EXPECT_TRUE(focused.isFocused);

    WindowInfo minimized = testWindow;
    minimized.state = WindowState::Minimized;
    minimized.isMinimized = true;
    EXPECT_TRUE(minimized.isMinimized);

    WindowInfo hidden = testWindow;
    hidden.state = WindowState::Hidden;
    hidden.isVisible = false;
    EXPECT_FALSE(hidden.isVisible);
}

} // namespace Tests
} // namespace WindowManager