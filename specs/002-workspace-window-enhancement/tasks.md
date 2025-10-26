# Implementation Tasks: Workspace Window Enhancement

**Feature**: Workspace Window Enhancement
**Branch**: `002-workspace-window-enhancement`
**Total Estimated Tasks**: 47 tasks across 6 phases
**MVP Scope**: User Story 1 (Enhanced Window Listing)

## Task Summary by Phase

- **Phase 1 (Setup)**: 5 tasks - Project initialization and dependency configuration
- **Phase 2 (Foundation)**: 8 tasks - Core data structures and platform detection
- **Phase 3 (User Story 1)**: 15 tasks - Enhanced window listing with workspace information
- **Phase 4 (User Story 2)**: 9 tasks - Extended search functionality
- **Phase 5 (User Story 3)**: 6 tasks - Cross-workspace window management
- **Phase 6 (Polish)**: 4 tasks - Performance optimization and documentation

---

## Phase 1: Setup & Project Initialization

**Goal**: Prepare the development environment and configure platform-specific dependencies for workspace integration.

### Setup Tasks

- [x] T001 Update CMakeLists.txt with enhanced platform-specific libraries per research.md
- [x] T002 Create platform detection headers in include/platform_config.h
- [x] T003 [P] Add Windows COM interface dependencies (shell32, ole32, oleaut32)
- [x] T004 [P] Add macOS Accessibility framework dependencies (ApplicationServices)
- [x] T005 [P] Add Linux X11 extended libraries (X11, Xext)

---

## Phase 2: Foundation & Core Structures

**Goal**: Establish enhanced data structures and base interfaces that support workspace functionality.

**Completion Criteria**: All core data structures implemented and platform enumerators can be instantiated.

### Foundation Tasks

- [x] T006 Define WindowState enumeration in src/core/window.hpp
- [x] T007 [P] Enhance WindowInfo struct with workspace fields in src/core/window.hpp
- [x] T008 [P] Create WorkspaceInfo structure in src/core/workspace.hpp
- [x] T009 [P] Implement enhanced WindowInfo constructors in src/core/window.cpp
- [x] T010 [P] Add WindowInfo workspace validation methods in src/core/window.cpp
- [x] T011 [P] Enhance WindowInfo JSON output with new fields in src/core/window.cpp
- [x] T012 Update base WindowEnumerator interface with workspace methods in src/core/enumerator.hpp
- [x] T013 [P] Create SearchField enumeration and SearchQuery structure in src/filters/search_query.hpp

---

## Phase 3: User Story 1 - Enhanced Window Listing with Workspace Information (P1)

**Story Goal**: Users can see all windows across different workspaces/desktops with clear visibility indicators showing window state (focused, minimized, workspace location).

**Independent Test**: Open windows across multiple workspaces, run enhanced list command, verify each window shows current state and workspace location.

**Acceptance Criteria**:
- All windows displayed with workspace identifiers and state information
- Window states clearly shown (focused/minimized/normal)
- Each window shows which workspace/desktop it belongs to

### Windows Platform Implementation

- [x] T014 [P] [US1] Initialize COM interfaces in Win32Enumerator constructor in src/platform/windows/win32_enumerator.cpp
- [x] T015 [P] [US1] Implement IVirtualDesktopManager integration in src/platform/windows/win32_enumerator.cpp
- [x] T016 [P] [US1] Add Windows workspace enumeration methods in src/platform/windows/win32_enumerator.cpp
- [x] T017 [US1] Enhance Windows window info creation with workspace data in src/platform/windows/win32_enumerator.cpp
- [x] T018 [US1] Implement Windows focus and minimized state detection in src/platform/windows/win32_enumerator.cpp

### macOS Platform Implementation

- [x] T019 [P] [US1] Add Accessibility permissions check in src/platform/macos/cocoa_enumerator.cpp
- [x] T020 [P] [US1] Implement Core Graphics workspace enumeration in src/platform/macos/cocoa_enumerator.cpp
- [x] T021 [P] [US1] Add macOS workspace detection using CGWindowListCopyWindowInfo in src/platform/macos/cocoa_enumerator.cpp
- [x] T022 [US1] Enhance macOS window info creation with workspace data in src/platform/macos/cocoa_enumerator.cpp
- [x] T023 [US1] Implement macOS focus detection via Accessibility API in src/platform/macos/cocoa_enumerator.cpp

### Linux Platform Implementation

- [x] T024 [P] [US1] Initialize EWMH atoms and X11 connection in src/platform/linux/x11_enumerator.cpp
- [x] T025 [P] [US1] Implement EWMH workspace enumeration (_NET_NUMBER_OF_DESKTOPS) in src/platform/linux/x11_enumerator.cpp
- [x] T026 [P] [US1] Add Linux workspace detection using _NET_WM_DESKTOP in src/platform/linux/x11_enumerator.cpp
- [x] T027 [US1] Enhance Linux window info creation with workspace data in src/platform/linux/x11_enumerator.cpp
- [x] T028 [US1] Implement Linux focus and minimized state detection via EWMH in src/platform/linux/x11_enumerator.cpp

### CLI Integration

- [x] T029 [US1] Update CLI list command to display workspace information in src/ui/cli.cpp
- [x] T030 [US1] Add workspace header formatting and window state indicators in src/ui/cli.cpp

---

## Phase 4: User Story 2 - Extended Search Functionality (P2)

**Story Goal**: Users can search for windows by both title and application name (owner), enabling them to find all windows belonging to a specific application regardless of individual window titles.

**Independent Test**: Open multiple windows from same application with different titles, search by application name, verify all relevant windows found.

**Acceptance Criteria**:
- Search returns all Chrome windows when searching "Chrome" regardless of individual titles
- Case-insensitive search works for partial application names
- Results include matches from both title and application name

### Search Enhancement Implementation

- [x] T031 [P] [US2] Implement SearchQuery validation and matching logic in src/filters/search_query.cpp
- [x] T032 [P] [US2] Add SearchQuery title and owner matching methods in src/filters/search_query.cpp
- [x] T033 [P] [US2] Create FilterResult structure with workspace grouping in src/filters/filter_result.hpp
- [x] T034 [P] [US2] Implement FilterResult grouping and statistics in src/filters/filter_result.cpp
- [x] T035 [P] [US2] Add FilterResult output formatting with workspace info in src/filters/filter_result.cpp
- [x] T036 [US2] Enhance existing filter logic to use SearchQuery in src/filters/filter.cpp
- [x] T037 [US2] Update CLI search command to support enhanced queries in src/ui/cli.cpp
- [x] T038 [US2] Add backward-compatible search parameter parsing in src/ui/cli.cpp
- [x] T039 [US2] Implement enhanced search result display with workspace grouping in src/ui/cli.cpp

---

## Phase 5: User Story 3 - Cross-Workspace Window Management (P3)

**Story Goal**: Users can identify and manage windows across different workspaces from a single view, enabling better organization and workflow management.

**Independent Test**: Use enhanced tools to locate and identify windows across multiple workspaces without manually switching between them.

**Acceptance Criteria**:
- Search/list results clearly indicate which workspace each window belongs to
- Currently focused window is clearly identified with its workspace
- Minimized state and workspace are both clearly shown

### Cross-Workspace Management Implementation

- [x] T040 [P] [US3] Add workspace filtering options to CLI commands in src/ui/cli.cpp
- [x] T041 [P] [US3] Implement workspace summary and status commands in src/ui/cli.cpp
- [x] T042 [P] [US3] Add focused window identification across all workspaces in src/core/window_manager.cpp
- [x] T043 [US3] Create workspace information display helpers in src/ui/cli.cpp
- [x] T044 [US3] Implement cross-workspace window statistics in src/filters/filter_result.cpp
- [x] T045 [US3] Add workspace-aware JSON output formatting in src/core/window.cpp

---

## Phase 6: Polish & Cross-Cutting Concerns

**Goal**: Optimize performance, complete documentation, and ensure production readiness.

### Polish Tasks

- [x] T046 [P] Add performance monitoring and caching for workspace enumeration in src/core/window_manager.cpp
- [x] T047 [P] Create comprehensive error handling and graceful degradation in src/core/exceptions.cpp
- [x] T048 [P] Update help documentation and CLI interface descriptions in src/ui/cli.cpp
- [x] T049 Validate backward compatibility with existing JSON output format across all components

---

## Dependencies & Execution Order

### User Story Dependencies
```
Setup (Phase 1) → Foundation (Phase 2) → User Stories (Phase 3,4,5) → Polish (Phase 6)

User Story 1 (P1) ← Foundation
User Story 2 (P2) ← User Story 1 (enhanced WindowInfo needed)
User Story 3 (P3) ← User Story 1 + User Story 2 (search and listing needed)
```

### Platform Implementation Dependencies
- Windows: T014 → T015 → T016 → T017 → T018
- macOS: T019 → T020 → T021 → T022 → T023
- Linux: T024 → T025 → T026 → T027 → T028

### Feature Dependencies
- Search Enhancement: T031,T032,T033 → T034,T035 → T036,T037,T038,T039
- Cross-Workspace: T040,T041,T042 → T043,T044,T045

---

## Parallel Execution Opportunities

### Phase 1 (Setup)
**Parallel Groups**:
- Group A: T001, T002 (core setup)
- Group B: T003, T004, T005 (platform dependencies - fully parallel)

### Phase 2 (Foundation)
**Parallel Groups**:
- Group A: T006, T012 (interface definitions)
- Group B: T007, T008, T013 (data structures - parallel by file)
- Group C: T009, T010, T011 (implementations - after Group A+B)

### Phase 3 (User Story 1)
**Parallel Groups**:
- Group A: T014,T015,T016 + T019,T020,T021 + T024,T025,T026 (platform workspace enumeration)
- Group B: T017,T018 + T022,T023 + T027,T028 (platform window enhancement - after Group A)
- Group C: T029, T030 (CLI integration - after Group B)

### Phase 4 (User Story 2)
**Parallel Groups**:
- Group A: T031, T032, T033, T034, T035 (search infrastructure - parallel by file)
- Group B: T036, T037, T038, T039 (integration - after Group A)

### Phase 5 (User Story 3)
**Parallel Groups**:
- Group A: T040, T041, T042 (workspace management - parallel by file)
- Group B: T043, T044, T045 (display and output - after Group A)

### Phase 6 (Polish)
**Parallel Groups**:
- Group A: T046, T047, T048 (parallel by file)
- Group B: T049 (validation - after Group A)

---

## MVP Implementation Strategy

### Recommended MVP Scope: User Story 1 Only
**Phases**: 1, 2, 3 (Total: 28 tasks)
**Deliverables**:
- Enhanced window listing with workspace information
- Cross-platform workspace enumeration
- Window state detection (focused/minimized)
- Backward-compatible CLI interface

### Incremental Delivery Plan
1. **MVP Release**: Phases 1-3 (User Story 1)
2. **Search Enhancement**: Phase 4 (User Story 2)
3. **Full Release**: Phase 5 (User Story 3) + Phase 6 (Polish)

### Testing Strategy (Per User Story)
- **User Story 1**: Open windows across multiple workspaces → run list command → verify workspace info and state indicators
- **User Story 2**: Open multiple Chrome windows with different titles → search "Chrome" → verify all found
- **User Story 3**: Create complex multi-workspace setup → use enhanced tools → verify cross-workspace visibility

---

## Implementation Notes

### Critical Success Factors
1. **Backward Compatibility**: All existing CLI commands must work unchanged
2. **Performance**: <3 seconds for window enumeration, <1 second for search
3. **Cross-Platform**: Consistent functionality across Windows, macOS, Linux
4. **Graceful Degradation**: Works on platforms without workspace support

### Platform-Specific Considerations
- **Windows**: COM interface initialization required before workspace operations
- **macOS**: Accessibility permissions required for enhanced focus detection
- **Linux**: EWMH compatibility varies by window manager - implement fallbacks

### Risk Mitigation
- Implement fallback modes for unsupported platforms/configurations
- Use existing window enumeration as baseline when workspace info unavailable
- Maintain existing JSON schema with additive fields only