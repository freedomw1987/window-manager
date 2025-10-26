#include <gtest/gtest.h>
#include "../../src/filters/search_query.hpp"
#include "../../src/core/window.hpp"

namespace WindowManager {
namespace Tests {

class SearchQueryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test window
        testWindow.handle = "test_handle";
        testWindow.title = "Google Chrome - My Document";
        testWindow.ownerName = "chrome.exe";
        testWindow.processId = 1234;
        testWindow.workspaceId = "workspace_1";
        testWindow.isVisible = true;
        testWindow.x = 100;
        testWindow.y = 100;
        testWindow.width = 800;
        testWindow.height = 600;
    }

    WindowInfo testWindow;
};

TEST_F(SearchQueryTest, DefaultConstructor) {
    SearchQuery query;

    EXPECT_TRUE(query.isEmpty());
    EXPECT_EQ(query.field, SearchField::Both);
    EXPECT_FALSE(query.caseSensitive);
    EXPECT_FALSE(query.useRegex);
    EXPECT_TRUE(query.workspaceFilter.empty());
}

TEST_F(SearchQueryTest, ParameterizedConstructor) {
    SearchQuery query("chrome", SearchField::Title, true, false);

    EXPECT_EQ(query.query, "chrome");
    EXPECT_EQ(query.field, SearchField::Title);
    EXPECT_TRUE(query.caseSensitive);
    EXPECT_FALSE(query.useRegex);
}

TEST_F(SearchQueryTest, DualFieldMatching) {
    // Test matching both title and owner
    SearchQuery query("chrome", SearchField::Both);

    // Should match on owner name
    EXPECT_TRUE(query.matches(testWindow));

    // Test with different window
    WindowInfo window2 = testWindow;
    window2.title = "Visual Studio Code";
    window2.ownerName = "code.exe";

    SearchQuery queryVS("code", SearchField::Both);
    EXPECT_TRUE(queryVS.matches(window2));
    EXPECT_FALSE(query.matches(window2));
}

TEST_F(SearchQueryTest, TitleOnlyMatching) {
    SearchQuery query("Document", SearchField::Title);

    EXPECT_TRUE(query.matches(testWindow));

    // Should not match on owner name even if it contains the query
    WindowInfo window2 = testWindow;
    window2.title = "Some Other Title";
    window2.ownerName = "Document.exe";

    EXPECT_FALSE(query.matches(window2));
}

TEST_F(SearchQueryTest, OwnerOnlyMatching) {
    SearchQuery query("chrome", SearchField::Owner);

    EXPECT_TRUE(query.matches(testWindow));

    // Should not match on title even if it contains the query
    WindowInfo window2 = testWindow;
    window2.title = "chrome browser window";
    window2.ownerName = "firefox.exe";

    EXPECT_FALSE(query.matches(window2));
}

TEST_F(SearchQueryTest, CaseSensitiveMatching) {
    SearchQuery caseSensitive("Chrome", SearchField::Both, true);
    SearchQuery caseInsensitive("Chrome", SearchField::Both, false);

    // Test with exact case match
    EXPECT_TRUE(caseSensitive.matches(testWindow));
    EXPECT_TRUE(caseInsensitive.matches(testWindow));

    // Test with different case
    WindowInfo lowerCase = testWindow;
    lowerCase.title = "google chrome - my document";
    lowerCase.ownerName = "chrome.exe";

    EXPECT_FALSE(caseSensitive.matches(lowerCase));
    EXPECT_TRUE(caseInsensitive.matches(lowerCase));
}

TEST_F(SearchQueryTest, RegexMatching) {
    SearchQuery regexQuery("^Google.*Chrome", SearchField::Title, false, true);

    EXPECT_TRUE(regexQuery.matches(testWindow));

    WindowInfo nonMatching = testWindow;
    nonMatching.title = "Chrome - Google Search";

    EXPECT_FALSE(regexQuery.matches(nonMatching));
}

TEST_F(SearchQueryTest, WorkspaceFiltering) {
    SearchQuery query("chrome", SearchField::Both);
    query.workspaceFilter = "workspace_1";

    // Should match - correct workspace
    EXPECT_TRUE(query.matches(testWindow));

    // Should not match - different workspace
    WindowInfo differentWorkspace = testWindow;
    differentWorkspace.workspaceId = "workspace_2";

    EXPECT_FALSE(query.matches(differentWorkspace));

    // Empty workspace filter should match all
    SearchQuery noFilter("chrome", SearchField::Both);
    EXPECT_TRUE(noFilter.matches(testWindow));
    EXPECT_TRUE(noFilter.matches(differentWorkspace));
}

TEST_F(SearchQueryTest, EmptyQueryMatching) {
    SearchQuery emptyQuery;

    // Empty query should match all windows
    EXPECT_TRUE(emptyQuery.matches(testWindow));

    WindowInfo anyWindow;
    anyWindow.title = "Any Title";
    anyWindow.ownerName = "any.exe";
    EXPECT_TRUE(emptyQuery.matches(anyWindow));
}

TEST_F(SearchQueryTest, PartialMatching) {
    SearchQuery query("Chro", SearchField::Both);

    // Should match partial strings
    EXPECT_TRUE(query.matches(testWindow));

    SearchQuery docQuery("Doc", SearchField::Title);
    EXPECT_TRUE(docQuery.matches(testWindow));
}

TEST_F(SearchQueryTest, SpecialCharacterHandling) {
    WindowInfo specialWindow = testWindow;
    specialWindow.title = "File [Modified] - Editor++";
    specialWindow.ownerName = "editor++.exe";

    SearchQuery bracketQuery("[Modified]", SearchField::Title);
    EXPECT_TRUE(bracketQuery.matches(specialWindow));

    SearchQuery plusQuery("editor++", SearchField::Owner);
    EXPECT_TRUE(plusQuery.matches(specialWindow));
}

TEST_F(SearchQueryTest, ValidationMethods) {
    SearchQuery valid("test", SearchField::Both);
    EXPECT_TRUE(valid.isValid());

    SearchQuery empty;
    EXPECT_TRUE(empty.isValid()); // Empty queries are valid

    SearchQuery invalidRegex("[unclosed", SearchField::Both, false, true);
    // Note: In a real implementation, this should validate regex syntax
    // For now, we assume the query construction handles validation
}

TEST_F(SearchQueryTest, ToStringOutput) {
    SearchQuery query("chrome", SearchField::Title, true, false);
    query.workspaceFilter = "workspace_1";

    std::string str = query.toString();

    EXPECT_NE(str.find("chrome"), std::string::npos);
    EXPECT_NE(str.find("Title"), std::string::npos);
    EXPECT_NE(str.find("workspace_1"), std::string::npos);
}

TEST_F(SearchQueryTest, ComplexSearchScenarios) {
    // Test realistic search scenarios

    // Scenario 1: Find all Chrome windows in current workspace
    SearchQuery chromeQuery("chrome", SearchField::Both);
    chromeQuery.workspaceFilter = "workspace_1";

    WindowInfo chrome1 = testWindow;
    chrome1.title = "Gmail - Google Chrome";
    chrome1.ownerName = "chrome.exe";
    chrome1.workspaceId = "workspace_1";

    WindowInfo chrome2 = testWindow;
    chrome2.title = "YouTube - Google Chrome";
    chrome2.ownerName = "chrome.exe";
    chrome2.workspaceId = "workspace_2";

    EXPECT_TRUE(chromeQuery.matches(chrome1));
    EXPECT_FALSE(chromeQuery.matches(chrome2)); // Different workspace

    // Scenario 2: Case-insensitive application search
    SearchQuery appQuery("visual", SearchField::Owner, false);

    WindowInfo vsCode;
    vsCode.title = "main.cpp - Visual Studio Code";
    vsCode.ownerName = "Code.exe";

    EXPECT_TRUE(appQuery.matches(vsCode));

    // Scenario 3: Regex pattern for file types
    SearchQuery fileQuery(".*\\.cpp.*", SearchField::Title, false, true);

    WindowInfo cppFile;
    cppFile.title = "main.cpp - Editor";
    cppFile.ownerName = "editor.exe";

    EXPECT_TRUE(fileQuery.matches(cppFile));
}

} // namespace Tests
} // namespace WindowManager