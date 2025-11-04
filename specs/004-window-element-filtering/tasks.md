# Tasks: Window-Specific Element Operations

**Input**: Design documents from `/specs/004-window-element-filtering/`
**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, contracts/

**Tests**: No test tasks included - not explicitly requested in feature specification

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## 📊 Implementation Status

- ✅ **Phase 1: Setup** - All infrastructure tasks completed (T001-T007)
- ✅ **Phase 2: Foundational** - All core enumeration infrastructure completed (T008-T018)
- ✅ **Phase 3: User Story 1** - MVP element listing completed (T019-T025) 🎯
- ✅ **Phase 4: User Story 2** - Element search within windows completed (T026-T032) 🎯
- ⏳ **Phase 5: User Story 3** - Enhanced error handling (T033-T039)
- ⏳ **Phase 6: Polish** - Cross-cutting improvements (T040-T049)

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

Using single project structure as defined in plan.md:
- Core source: `src/`
- Tests: `tests/`
- Headers: `include/`

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and element enumeration infrastructure

- [x] T001 Create UIElement data structures in src/core/ui_element.hpp
- [x] T002 Create UIElement implementation in src/core/ui_element.cpp
- [x] T003 [P] Create ElementType and ElementState enums in src/core/element_types.hpp
- [x] T004 [P] Create ElementEnumerationResult structure in src/core/element_result.hpp
- [x] T005 [P] Create ElementSearchQuery structure in src/core/element_query.hpp
- [x] T006 Implement ElementEnumerationResult in src/core/element_result.cpp
- [x] T007 Implement ElementSearchQuery in src/core/element_query.cpp

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core element enumeration infrastructure that MUST be complete before ANY user story can be implemented

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [x] T008 Create base ElementEnumerator interface in src/core/element_enumerator.hpp
- [x] T009 Create ElementEnumerator factory implementation in src/core/element_enumerator.cpp
- [x] T010 [P] Create Windows element enumerator in src/platform/windows/win32_element_enumerator.hpp
- [x] T011 [P] Create macOS element enumerator in src/platform/macos/cocoa_element_enumerator.hpp
- [x] T012 [P] Create Linux element enumerator in src/platform/linux/x11_element_enumerator.hpp
- [x] T013 Implement Windows element enumeration in src/platform/windows/win32_element_enumerator.cpp
- [x] T014 Implement macOS element enumeration in src/platform/macos/cocoa_element_enumerator.cpp
- [x] T015 Implement Linux element enumeration in src/platform/linux/x11_element_enumerator.cpp
- [x] T016 Extend WindowInfo with element enumeration support in src/core/window.hpp
- [x] T017 Create ElementCLI interface for element display in src/ui/element_cli.hpp
- [x] T018 Implement ElementCLI functionality in src/ui/element_cli.cpp

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel

---

## Phase 3: User Story 1 - List Elements in Specific Window (Priority: P1) 🎯 MVP ✅ COMPLETED

**Goal**: Enable users to list all UI elements within a specific application window using `list --window <handle>`

**Independent Test**: Run `list --window <handle>` with valid window handle and verify only elements from that window are returned

### Implementation for User Story 1

- [x] T019 [US1] Add --window parameter parsing to main.cpp argument handling
- [x] T020 [US1] Modify listWindows function signature to accept windowHandle parameter in src/main.cpp
- [x] T021 [US1] Implement element enumeration logic in modified listWindows function
- [x] T022 [US1] Add element validation and error handling for invalid window handles
- [x] T023 [US1] Integrate ElementCLI for element display output
- [x] T024 [US1] Add element enumeration performance tracking and reporting
- [x] T025 [US1] Update help text and usage examples to include --window parameter

**Checkpoint**: ✅ User Story 1 is fully functional and testable independently - MVP COMPLETE!

---

## Phase 4: User Story 2 - Search Elements Within Specific Window (Priority: P2) ✅ COMPLETED

**Goal**: Enable users to search for specific UI elements within a window using `search --window <handle> <criteria>`

**Independent Test**: Run `search --window <handle> <criteria>` and verify results are limited to specified window and match search terms

### Implementation for User Story 2

- [x] T026 [US2] Add --window parameter parsing to search command in main.cpp
- [x] T027 [US2] Modify searchWindows function signature to accept windowHandle parameter
- [x] T028 [US2] Implement element search logic using ElementSearchQuery
- [x] T029 [US2] Add element filtering and matching algorithms
- [x] T030 [US2] Integrate element search results with existing CLI output
- [x] T031 [US2] Add search performance optimization for window-scoped operations
- [x] T032 [US2] Update search help documentation with window parameter examples

**Checkpoint**: ✅ User Stories 1 AND 2 both work independently - Element search functionality complete!

---

## Phase 5: User Story 3 - Handle Invalid Window References (Priority: P3)

**Goal**: Provide clear error messages and graceful handling when users provide invalid window handles

**Independent Test**: Test with invalid window handles and verify appropriate error messages are displayed

### Implementation for User Story 3

- [ ] T033 [US3] Create comprehensive window handle validation in src/core/window_validator.hpp
- [ ] T034 [US3] Implement window handle validation logic in src/core/window_validator.cpp
- [ ] T035 [US3] Add user-friendly error messages for different error scenarios
- [ ] T036 [US3] Implement graceful degradation when element access fails
- [ ] T037 [US3] Add platform-specific error guidance and troubleshooting
- [ ] T038 [US3] Create error message localization framework for better UX
- [ ] T039 [US3] Add comprehensive error logging for debugging support

**Checkpoint**: All user stories should now be independently functional

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories

- [ ] T040 [P] Add element enumeration examples to quickstart.md documentation
- [ ] T041 [P] Optimize element caching across all window operations
- [ ] T042 [P] Add element enumeration performance benchmarking
- [ ] T043 [P] Implement element cache management and cleanup
- [ ] T044 [P] Add comprehensive platform capability detection
- [ ] T045 Code cleanup and consistency improvements across all element files
- [ ] T046 Add memory usage optimization for large element collections
- [ ] T047 Validate all quickstart.md examples work correctly
- [ ] T048 [P] Add element enumeration to interactive UI mode
- [ ] T049 [P] Create element enumeration developer documentation

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
- **User Story 2 (P2)**: Can start after Foundational (Phase 2) - Extends US1 functionality but independently testable
- **User Story 3 (P3)**: Can start after Foundational (Phase 2) - Enhances error handling for US1/US2 but independently testable

### Within Each User Story

- Command-line argument parsing before function implementation
- Core logic before UI integration
- Error handling before performance optimization
- Story complete before moving to next priority

### Parallel Opportunities

- All Setup tasks marked [P] can run in parallel (different header files)
- All Foundational platform-specific tasks (T010-T015) can run in parallel
- Once Foundational phase completes, all user stories can start in parallel (if team capacity allows)
- Documentation and optimization tasks in Polish phase can run in parallel

---

## Parallel Example: User Story 1

```bash
# These tasks can start together after Foundational phase:
Task: "Add --window parameter parsing to main.cpp argument handling"
Task: "Add element enumeration performance tracking and reporting"
Task: "Update help text and usage examples to include --window parameter"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only) ✅ COMPLETED

1. ✅ Complete Phase 1: Setup (T001-T007)
2. ✅ Complete Phase 2: Foundational (T008-T018) - CRITICAL - blocks all stories
3. ✅ Complete Phase 3: User Story 1 (T019-T025)
4. ✅ **VALIDATED**: Test `list --window <handle>` independently - ALL TESTS PASS
5. ✅ Ready for deploy/demo element listing functionality

### Incremental Delivery

1. Complete Setup + Foundational → Element enumeration infrastructure ready
2. Add User Story 1 → Test `list --window` independently → Deploy/Demo (MVP!)
3. Add User Story 2 → Test `search --window` independently → Deploy/Demo
4. Add User Story 3 → Test error handling independently → Deploy/Demo
5. Each story adds value without breaking previous stories

### Parallel Team Strategy

With multiple developers:

1. Team completes Setup + Foundational together
2. Once Foundational is done:
   - Developer A: User Story 1 (element listing)
   - Developer B: User Story 2 (element search)
   - Developer C: User Story 3 (error handling)
3. Stories complete and integrate independently

---

## Platform-Specific Considerations

### Windows Implementation (T013)
- Use UI Automation (IUIAutomation) APIs
- Handle COM initialization and cleanup
- Implement element tree traversal
- Add Windows-specific error handling

### macOS Implementation (T014)
- Use Accessibility APIs (AXUIElement)
- Verify accessibility permissions
- Handle Core Foundation memory management
- Implement macOS-specific element discovery

### Linux Implementation (T015)
- Use AT-SPI2 or X11 property inspection
- Handle variable application support
- Implement fallback mechanisms
- Add X11-specific error handling

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- Each user story should be independently completable and testable
- Focus on backward compatibility - existing commands must continue working
- Commit after each task or logical group
- Stop at any checkpoint to validate story independently
- Performance target: < 2 seconds for 100 element enumeration
- Memory target: < 50MB for element cache