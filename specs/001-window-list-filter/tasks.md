# Tasks: Window List and Filter Program

**Input**: Design documents from `/specs/001-window-list-filter/`
**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, contracts/

**Tests**: Tests are NOT explicitly requested in the feature specification, so test tasks are omitted.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and basic structure

- [X] T001 Create project structure per implementation plan in src/, include/, tests/, CMakeLists.txt
- [X] T002 Initialize C++17 project with CMake 3.16+ configuration in CMakeLists.txt
- [X] T003 [P] Configure build system for cross-platform support (Windows/Linux/macOS) in CMakeLists.txt
- [X] T004 [P] Setup Google Test dependency and test framework configuration in CMakeLists.txt
- [X] T005 [P] Setup FTXUI dependency for terminal UI in CMakeLists.txt

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [X] T006 Create WindowInfo data structure in src/core/window.hpp and src/core/window.cpp
- [X] T007 [P] Create abstract WindowEnumerator interface in src/core/enumerator.hpp
- [X] T008 [P] Create platform detection and factory methods in include/platform_config.h
- [X] T009 [P] Create platform-specific directory structure in src/platform/windows/, src/platform/linux/, src/platform/macos/
- [X] T010 [P] Setup error handling and exception classes in src/core/exceptions.hpp and src/core/exceptions.cpp
- [X] T011 [P] Create main application entry point structure in src/main.cpp

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel

---

## Phase 3: User Story 1 - Basic Window Listing (Priority: P1) 🎯 MVP

**Goal**: Enable users to see all currently open windows with their basic information including position, size, and title

**Independent Test**: Run the program and verify it displays a complete list of all visible windows with their positions and delivers immediate value for window management

### Implementation for User Story 1

- [X] T012 [P] [US1] Implement Windows-specific enumeration in src/platform/windows/win32_enumerator.hpp and src/platform/windows/win32_enumerator.cpp
- [X] T013 [P] [US1] Implement Linux X11-specific enumeration in src/platform/linux/x11_enumerator.hpp and src/platform/linux/x11_enumerator.cpp
- [X] T014 [P] [US1] Implement macOS Core Graphics enumeration in src/platform/macos/cocoa_enumerator.hpp and src/platform/macos/cocoa_enumerator.cpp
- [X] T015 [US1] Implement concrete WindowEnumerator factory in src/core/enumerator.cpp
- [X] T016 [US1] Create basic CLI output formatter in src/ui/cli.hpp and src/ui/cli.cpp
- [X] T017 [US1] Implement WindowManager facade class in src/core/window_manager.hpp and src/core/window_manager.cpp
- [X] T018 [US1] Integrate window enumeration with main application in src/main.cpp
- [X] T019 [US1] Add error handling for platform-specific failures and permission issues
- [X] T020 [US1] Add basic command line argument parsing for list command

**Checkpoint**: At this point, User Story 1 should be fully functional and testable independently

---

## Phase 4: User Story 2 - Keyword Filtering (Priority: P2)

**Goal**: Enable users to search for specific windows by entering keywords that match against window titles

**Independent Test**: Open multiple windows with different titles, enter various keywords, and verify only matching windows are displayed

### Implementation for User Story 2

- [X] T021 [P] [US2] Create SearchQuery data structure in src/filters/search_query.hpp and src/filters/search_query.cpp
- [X] T022 [P] [US2] Create FilterResult data structure in src/filters/filter_result.hpp and src/filters/filter_result.cpp
- [X] T023 [P] [US2] Implement WindowFilter interface in src/filters/filter.hpp
- [X] T024 [US2] Implement concrete filtering algorithm with case-insensitive matching in src/filters/filter.cpp
- [X] T025 [US2] Add search command line argument parsing in src/ui/cli.cpp
- [X] T026 [US2] Integrate filtering functionality with WindowManager in src/core/window_manager.cpp
- [X] T027 [US2] Add filtered results output formatting in src/ui/cli.cpp
- [X] T028 [US2] Add performance timing and metrics for search operations
- [X] T029 [US2] Add graceful handling for no matches found scenarios

**Checkpoint**: At this point, User Stories 1 AND 2 should both work independently

---

## Phase 5: User Story 3 - Interactive Filtering Interface (Priority: P3)

**Goal**: Enable users to continuously refine their search by entering different keywords without restarting the program

**Independent Test**: Run the program, enter multiple different search terms sequentially, and verify the results update appropriately for each search

### Implementation for User Story 3

- [X] T030 [P] [US3] Implement FTXUI-based interactive terminal interface in src/ui/interactive.hpp and src/ui/interactive.cpp
- [X] T031 [P] [US3] Add real-time input handling and search update logic in src/ui/interactive.cpp
- [X] T032 [US3] Implement background window list caching and refresh in src/core/window_manager.cpp
- [X] T033 [US3] Add interactive mode command line option parsing in src/ui/cli.cpp
- [X] T034 [US3] Integrate interactive mode with main application loop in src/main.cpp
- [X] T035 [US3] Add real-time display updates and result formatting in src/ui/interactive.cpp
- [X] T036 [US3] Add quit command handling and graceful exit in src/ui/interactive.cpp
- [X] T037 [US3] Optimize search performance for real-time filtering (<1 second requirement)

**Checkpoint**: All user stories should now be independently functional

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories

- [X] T038 [P] Add comprehensive error messages and user guidance for platform-specific issues
- [X] T039 [P] Add JSON output format support across all commands in src/ui/cli.cpp
- [X] T040 [P] Add verbose logging and debugging options
- [X] T041 [P] Performance optimization for 50+ window scenarios across all enumeration code
- [X] T042 [P] Add memory management optimization and leak prevention
- [X] T043 [P] Create comprehensive README.md with usage examples and build instructions
- [X] T044 Code cleanup and refactoring across all source files
- [X] T045 Cross-platform build testing and validation
- [X] T046 Run quickstart.md validation and ensure all examples work

**Status**: ✅ **COMPLETE** - All Phase 6 tasks implemented successfully

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion - BLOCKS all user stories
- **User Stories (Phase 3+)**: All depend on Foundational phase completion
  - User stories can then proceed in parallel (if staffed)
  - Or sequentially in priority order (P1 → P2 → P3)
- **Polish (Final Phase)**: Depends on all desired user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Can start after Foundational (Phase 2) - No dependencies on other stories
- **User Story 2 (P2)**: Can start after Foundational (Phase 2) - Builds on US1 but should be independently testable
- **User Story 3 (P3)**: Can start after Foundational (Phase 2) - Builds on US1 and US2 but should be independently testable

### Within Each User Story

- Platform-specific implementations (T012, T013, T014) can run in parallel
- Core data structures (T021, T022, T023) can run in parallel
- UI components can be developed in parallel with core logic
- Integration tasks must wait for dependencies to complete

### Parallel Opportunities

- All Setup tasks marked [P] can run in parallel
- All Foundational tasks marked [P] can run in parallel (within Phase 2)
- Platform-specific enumerators (Windows/Linux/macOS) can be developed simultaneously
- Data structures within each user story can be developed in parallel
- Different user stories can be worked on in parallel by different team members

---

## Parallel Example: User Story 1

```bash
# Launch platform-specific implementations together:
Task: "Implement Windows-specific enumeration in src/platform/windows/win32_enumerator.hpp and src/platform/windows/win32_enumerator.cpp"
Task: "Implement Linux X11-specific enumeration in src/platform/linux/x11_enumerator.hpp and src/platform/linux/x11_enumerator.cpp"
Task: "Implement macOS Core Graphics enumeration in src/platform/macos/cocoa_enumerator.hpp and src/platform/macos/cocoa_enumerator.cpp"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational (CRITICAL - blocks all stories)
3. Complete Phase 3: User Story 1
4. **STOP and VALIDATE**: Test User Story 1 independently
5. Deploy/demo if ready

### Incremental Delivery

1. Complete Setup + Foundational → Foundation ready
2. Add User Story 1 → Test independently → Deploy/Demo (MVP!)
3. Add User Story 2 → Test independently → Deploy/Demo
4. Add User Story 3 → Test independently → Deploy/Demo
5. Each story adds value without breaking previous stories

### Parallel Team Strategy

With multiple developers:

1. Team completes Setup + Foundational together
2. Once Foundational is done:
   - Developer A: User Story 1 (focus on one platform initially)
   - Developer B: User Story 2 (filtering logic)
   - Developer C: User Story 3 (interactive UI)
3. Stories complete and integrate independently

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- Each user story should be independently completable and testable
- Platform-specific code is isolated to enable parallel development
- Cross-platform compatibility maintained through abstract interfaces
- Performance requirements (3s enumeration, 1s search) addressed in relevant tasks
- Stop at any checkpoint to validate story independently
- Avoid: vague tasks, same file conflicts, cross-story dependencies that break independence