# Tasks: Window Focus by Handle

**Input**: Design documents from `/specs/003-window-focus-by-handle/`
**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, contracts/

**Tests**: Tests are NOT explicitly requested in the feature specification, so test tasks are omitted.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

- **Single project**: `src/`, `tests/` at repository root
- Paths align with existing codebase structure from plan.md

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and basic structure extensions for focus functionality

- [x] T001 [P] Add FocusRequest structure to src/core/focus_request.hpp
- [x] T002 [P] Add FocusOperation structure to src/core/focus_operation.hpp
- [x] T003 [P] Add FocusStatus enumeration to src/core/focus_status.hpp
- [x] T004 [P] Add workspace switching exceptions to src/core/exceptions.hpp
- [x] T005 [P] Add WorkspaceSwitchOperation structure to src/core/workspace_switch_operation.hpp

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [x] T006 Extend WindowInfo struct with focus-specific fields in src/core/window.hpp
- [x] T007 Add focusWindowByHandle method declaration to WindowManager in src/core/window_manager.hpp
- [x] T008 Add switchToWorkspace method declarations to platform enumerators in src/core/enumerator.hpp
- [x] T009 [P] Add focus command display methods to CLI in src/ui/cli.hpp
- [x] T010 [P] Extend CLI argument parsing for focus commands in src/ui/cli.cpp
- [x] T011 [P] Add focus operation error codes and mappings to src/core/exceptions.cpp

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel

---

## Phase 3: User Story 1 - Direct Window Focus in Current Workspace (Priority: P1) 🎯 MVP

**Goal**: Enable users to focus windows in their current workspace by providing window handle, with immediate response under 1 second

**Independent Test**: Provide valid window handle of window in current workspace and verify window receives focus within 1 second

### Implementation for User Story 1

- [x] T012 [P] [US1] Implement enhanced handle validation in src/platform/windows/win32_enumerator.cpp
- [x] T013 [P] [US1] Implement enhanced handle validation in src/platform/macos/cocoa_enumerator.cpp
- [x] T014 [P] [US1] Implement enhanced handle validation in src/platform/linux/x11_enumerator.cpp
- [x] T015 [US1] Extend WindowManager::focusWindowByHandle for current workspace in src/core/window_manager.cpp
- [x] T016 [US1] Add focus command parsing for 'focus <handle>' in src/ui/cli.cpp
- [x] T017 [US1] Implement CLI::displayFocusSuccess method in src/ui/cli.cpp
- [x] T018 [US1] Implement CLI::displayFocusError method in src/ui/cli.cpp
- [x] T019 [US1] Add focus command to main argument parsing in src/main.cpp
- [x] T020 [US1] Add performance timing for current workspace focus in src/core/window_manager.cpp

**Checkpoint**: At this point, User Story 1 should be fully functional and testable independently

---

## Phase 4: User Story 2 - Cross-Workspace Window Focus (Priority: P2)

**Goal**: Enable users to focus windows in different workspaces with automatic workspace switching, completing within 2 seconds

**Independent Test**: Create windows in different workspaces, provide handle from another workspace, verify workspace switch and window focus occur within 2 seconds

### Implementation for User Story 2

- [x] T021 [P] [US2] Implement workspace switching for Windows in src/platform/windows/win32_enumerator.cpp
- [x] T022 [P] [US2] Implement workspace switching for macOS in src/platform/macos/cocoa_enumerator.cpp
- [x] T023 [P] [US2] Implement workspace switching for Linux in src/platform/linux/x11_enumerator.cpp
- [x] T024 [US2] Extend platform focusWindow methods to detect cross-workspace needs in src/platform/windows/win32_enumerator.cpp
- [x] T025 [US2] Extend platform focusWindow methods to detect cross-workspace needs in src/platform/macos/cocoa_enumerator.cpp
- [x] T026 [US2] Extend platform focusWindow methods to detect cross-workspace needs in src/platform/linux/x11_enumerator.cpp
- [x] T027 [US2] Implement cross-workspace logic in WindowManager::focusWindowByHandle in src/core/window_manager.cpp
- [x] T028 [US2] Add workspace switching feedback to CLI success messages in src/ui/cli.cpp
- [x] T029 [US2] Add --no-workspace-switch option parsing in src/ui/cli.cpp
- [x] T030 [US2] Add performance timing for cross-workspace operations in src/core/window_manager.cpp

**Checkpoint**: At this point, User Stories 1 AND 2 should both work independently

---

## Phase 5: User Story 3 - Handle Input Validation and Error Handling (Priority: P3)

**Goal**: Provide clear error messages and validation feedback for invalid window handles within 0.5 seconds

**Independent Test**: Provide various invalid handles and verify appropriate error responses within 0.5 seconds

### Implementation for User Story 3

- [x] T031 [P] [US3] Add comprehensive handle format validation in src/platform/windows/win32_enumerator.cpp
- [x] T032 [P] [US3] Add comprehensive handle format validation in src/platform/macos/cocoa_enumerator.cpp
- [x] T033 [P] [US3] Add comprehensive handle format validation in src/platform/linux/x11_enumerator.cpp
- [x] T034 [US3] Implement validate-handle command in src/ui/cli.cpp
- [x] T035 [US3] Add detailed error message generation for different failure scenarios in src/core/exceptions.cpp
- [x] T036 [US3] Add timeout handling for validation operations in src/core/window_manager.cpp
- [x] T037 [US3] Implement CLI error display with suggestions in src/ui/cli.cpp
- [x] T038 [US3] Add JSON output support for error messages in src/ui/cli.cpp
- [x] T039 [US3] Add rate limiting for focus requests in src/core/window_manager.cpp

**Checkpoint**: All user stories should now be independently functional

---

## Phase 6: Enhanced Commands and User Experience

**Purpose**: Additional commands and features that improve the overall user experience

- [x] T040 [P] Extend list command with --show-handles option in src/ui/cli.cpp
- [x] T041 [P] Add --handles-only option for compact handle listing in src/ui/cli.cpp
- [x] T042 [P] Add --verbose option for detailed focus operation progress in src/ui/cli.cpp
- [x] T043 [P] Add --timeout option for custom operation timeouts in src/ui/cli.cpp
- [x] T044 [P] Add --json output format for all focus commands in src/ui/cli.cpp
- [x] T045 [P] Implement FocusOperation tracking and history in src/core/window_manager.cpp

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories and overall system quality

- [x] T046 [P] Update CMakeLists.txt to include new source files
- [x] T047 [P] Add performance monitoring integration for all focus operations in src/core/window_manager.cpp
- [x] T048 [P] Add graceful degradation when workspace switching unavailable in src/core/window_manager.cpp
- [x] T049 [P] Enhance error logging for all focus operations in src/core/window_manager.cpp
- [x] T050 [P] Add memory management for FocusOperation history in src/core/window_manager.cpp
- [x] T051 Run quickstart.md validation across all platforms
- [x] T052 Validate performance requirements across all user stories

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion - BLOCKS all user stories
- **User Stories (Phase 3-5)**: All depend on Foundational phase completion
  - User stories can then proceed in parallel (if staffed)
  - Or sequentially in priority order (P1 → P2 → P3)
- **Enhanced Commands (Phase 6)**: Depends on User Stories 1-3 completion
- **Polish (Phase 7)**: Depends on all desired user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Can start after Foundational (Phase 2) - No dependencies on other stories
- **User Story 2 (P2)**: Can start after Foundational (Phase 2) - Extends US1 focus logic but independently testable
- **User Story 3 (P3)**: Can start after Foundational (Phase 2) - Enhances validation but independently testable

### Within Each User Story

- Platform-specific implementations (marked [P]) can run in parallel
- Core WindowManager changes depend on platform enumerator extensions
- CLI extensions depend on core functionality being ready
- Error handling and performance timing can be added incrementally

### Parallel Opportunities

- All Setup tasks marked [P] can run in parallel
- All Foundational tasks marked [P] can run in parallel (within Phase 2)
- Once Foundational phase completes, all user stories can start in parallel (if team capacity allows)
- Platform-specific implementations within each story marked [P] can run in parallel
- Enhanced command features marked [P] can run in parallel
- Polish tasks marked [P] can run in parallel

---

## Parallel Example: User Story 1

```bash
# Launch all platform handle validations for User Story 1 together:
Task: "Implement enhanced handle validation in src/platform/windows/win32_enumerator.cpp"
Task: "Implement enhanced handle validation in src/platform/macos/cocoa_enumerator.cpp"
Task: "Implement enhanced handle validation in src/platform/linux/x11_enumerator.cpp"
```

---

## Parallel Example: User Story 2

```bash
# Launch all workspace switching implementations together:
Task: "Implement workspace switching for Windows in src/platform/windows/win32_enumerator.cpp"
Task: "Implement workspace switching for macOS in src/platform/macos/cocoa_enumerator.cpp"
Task: "Implement workspace switching for Linux in src/platform/linux/x11_enumerator.cpp"
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
   - Developer A: User Story 1 (focus in current workspace)
   - Developer B: User Story 2 (cross-workspace focus)
   - Developer C: User Story 3 (validation and error handling)
3. Stories complete and integrate independently

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- Each user story should be independently completable and testable
- Focus on extending existing infrastructure rather than creating new components
- Maintain performance requirements: <1s current workspace, <2s cross-workspace, <0.5s validation
- Platform-specific implementations follow existing patterns in codebase
- CLI extensions build on existing argument parsing and display methods
- Error handling leverages existing exception framework