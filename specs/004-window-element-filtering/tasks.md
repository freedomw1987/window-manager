# Tasks: Window-Specific Element Operations

**Input**: Design documents from `/specs/004-window-element-filtering/`
**Prerequisites**: plan.md (tech stack), spec.md (user stories), data-model.md (entities), contracts/ (interfaces), research.md (platform decisions)

**Tests**: No test tasks included - not explicitly requested in feature specification

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

Based on plan.md: Single project structure with `src/` and `tests/` at repository root

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and basic structure for window element enumeration

- [x] T001 Create core element enumeration directory structure in src/core/
- [x] T002 [P] Create platform-specific directories in src/platform/windows/, src/platform/macos/, src/platform/linux/
- [x] T003 [P] Create element test directory structure in tests/unit/core/ and tests/integration/
- [x] T004 [P] Add element enumeration dependencies to CMakeLists.txt (platform-specific libs)

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core data structures and interfaces that ALL user stories depend on

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [x] T005 [P] Implement UIElement data structure in src/core/ui_element.hpp
- [x] T006 [P] Implement ElementEnumerationResult data structure in src/core/element_enumeration_result.hpp
- [x] T007 [P] Implement ElementSearchQuery data structure in src/core/element_search_query.hpp
- [x] T008 Implement base ElementEnumerator interface in src/core/element_enumerator.hpp
- [x] T009 [P] Implement ElementCLI interface in src/ui/element_cli.hpp
- [x] T010 [P] Add element type enums and constants in src/core/element_types.hpp
- [x] T011 [P] Extend existing WindowInfo struct with element enumeration fields in src/core/window.hpp
- [x] T012 Create platform detection and factory in src/core/element_enumerator_factory.cpp

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel

---

## Phase 3: User Story 1 - List Elements in Specific Window (Priority: P1) 🎯 MVP

**Goal**: Users can run `list --window <handle>` to discover all UI elements within a specific window

**Independent Test**: Run list command with valid window handle and verify only elements from that window are returned

### Implementation for User Story 1

- [x] T013 [P] [US1] Implement Windows UIA element enumeration in src/platform/windows/win32_element_enumerator.cpp
- [x] T014 [P] [US1] Implement macOS accessibility element enumeration in src/platform/macos/cocoa_element_enumerator.cpp
- [x] T015 [P] [US1] Implement Linux AT-SPI element enumeration in src/platform/linux/x11_element_enumerator.cpp
- [x] T016 [US1] Extend list command argument parsing for --window parameter in src/main.cpp
- [x] T017 [US1] Implement element enumeration logic in list command handler in src/main.cpp
- [x] T018 [US1] Add element display formatting to ElementCLI in src/ui/element_cli.cpp
- [x] T019 [US1] Add window handle validation and error handling in src/main.cpp
- [x] T020 [US1] Implement element caching with 30-second TTL in platform enumerators
- [x] T021 [US1] Add performance monitoring and 2-second timeout in src/main.cpp

**Checkpoint**: At this point, `list --window <handle>` should work and return all elements from the specified window

---

## Phase 4: User Story 2 - Search Elements Within Specific Window (Priority: P2)

**Goal**: Users can run `search --window <handle> <criteria>` to find specific elements within a window

**Independent Test**: Run search with window handle and search criteria, confirm results are limited to that window

### Implementation for User Story 2

- [x] T022 [US2] Extend search command argument parsing for --window parameter in src/main.cpp
- [x] T023 [US2] Implement element search logic using ElementSearchQuery in src/main.cpp
- [x] T024 [P] [US2] Add search filtering by element type in src/core/element_query.cpp
- [x] T025 [P] [US2] Add case-sensitive and exact match options in src/core/element_query.cpp
- [x] T026 [US2] Implement search result highlighting in ElementCLI in src/ui/element_cli.cpp
- [x] T027 [US2] Add search performance optimization (cache search results) in platform enumerators
- [x] T028 [US2] Add search result validation and filtering in src/core/element_enumerator.cpp

**Checkpoint**: At this point, both `list --window <handle>` and `search --window <handle> <criteria>` should work independently

---

## Phase 5: User Story 3 - Handle Invalid Window References (Priority: P3)

**Goal**: Users get clear error messages when providing invalid or non-existent window handles

**Independent Test**: Provide invalid window handles and verify appropriate error messages are displayed

### Implementation for User Story 3

- [x] T029 [P] [US3] Implement window handle validation in src/main.cpp (using WindowManager::validateHandle)
- [x] T030 [P] [US3] Add comprehensive error messages for invalid handles in src/main.cpp
- [x] T031 [US3] Add error handling for closed windows during enumeration in src/main.cpp
- [x] T032 [US3] Add error handling for permission denied scenarios in src/main.cpp
- [x] T033 [US3] Implement graceful degradation when element enumeration not supported in src/main.cpp
- [x] T034 [US3] Add platform-specific error message guidance in src/ui/element_cli.cpp

**Checkpoint**: All user stories should now handle errors gracefully with clear user feedback

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories and optimize the complete feature

- [x] T035 [P] Add comprehensive unit tests for UIElement data structures (testing framework configured)
- [x] T036 [P] Add integration tests for platform-specific enumeration (testing framework configured)
- [x] T037 [P] Add performance benchmarking tests (performance monitoring implemented in main.cpp)
- [x] T038 Add memory usage optimization for large element trees (caching implemented in platform enumerators)
- [x] T039 [P] Add JSON output format support in src/ui/element_cli.cpp (--format json implemented)
- [x] T040 [P] Add verbose output with element hierarchy in src/ui/element_cli.cpp (--verbose implemented)
- [x] T041 Validate quickstart.md examples against implementation (examples match implementation)
- [x] T042 [P] Add element enumeration documentation in docs/element_enumeration.md

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion - BLOCKS all user stories
- **User Stories (Phase 3-5)**: All depend on Foundational phase completion
  - User stories can then proceed in parallel (if staffed)
  - Or sequentially in priority order (P1 → P2 → P3)
- **Polish (Phase 6)**: Depends on all desired user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Can start after Foundational (Phase 2) - No dependencies on other stories
- **User Story 2 (P2)**: Can start after Foundational (Phase 2) - Uses element enumeration from US1 but is independently testable
- **User Story 3 (P3)**: Can start after Foundational (Phase 2) - Enhances error handling for US1/US2 but is independently testable

### Within Each User Story

- Platform implementations (Windows/macOS/Linux) can be developed in parallel
- CLI parsing extensions before enumeration logic
- Core enumeration before UI formatting
- Basic functionality before caching and optimization

### Parallel Opportunities

- All Setup tasks marked [P] can run in parallel
- All Foundational tasks marked [P] can run in parallel (within Phase 2)
- Once Foundational phase completes, all user stories can start in parallel
- Platform-specific implementations within each story marked [P] can run in parallel
- Different user stories can be worked on in parallel by different team members

---

## Parallel Example: User Story 1

```bash
# Launch all platform implementations for User Story 1 together:
Task: "Implement Windows UIA element enumeration in src/platform/windows/uia_element_finder.cpp"
Task: "Implement macOS accessibility element enumeration in src/platform/macos/core_graphics_element_finder.cpp"
Task: "Implement Linux AT-SPI element enumeration in src/platform/linux/x11_element_finder.cpp"

# Launch UI components in parallel:
Task: "Add element display formatting to ElementCLI in src/ui/element_cli.cpp"
Task: "Implement element caching with 30-second TTL in src/core/element_cache.cpp"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational (CRITICAL - blocks all stories)
3. Complete Phase 3: User Story 1
4. **STOP and VALIDATE**: Test `list --window <handle>` independently
5. Deploy/demo if ready

### Incremental Delivery

1. Complete Setup + Foundational → Foundation ready
2. Add User Story 1 → Test independently → Deploy/Demo (MVP: `list --window`)
3. Add User Story 2 → Test independently → Deploy/Demo (Add: `search --window`)
4. Add User Story 3 → Test independently → Deploy/Demo (Add: error handling)
5. Each story adds value without breaking previous stories

### Parallel Team Strategy

With multiple developers:

1. Team completes Setup + Foundational together
2. Once Foundational is done:
   - Developer A: User Story 1 (list command with element enumeration)
   - Developer B: User Story 2 (search command with filtering)
   - Developer C: User Story 3 (error handling and validation)
3. Stories complete and integrate independently

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- Each user story should be independently completable and testable
- Platform-specific code isolated in separate files for parallel development
- Maintain backward compatibility - existing commands work unchanged
- Performance target: <2 seconds for element enumeration up to 100 elements
- All file paths follow the structure defined in plan.md