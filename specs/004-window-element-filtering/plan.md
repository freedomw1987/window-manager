# Implementation Plan: Window-Specific Element Operations

**Branch**: `004-window-element-filtering` | **Date**: 2025-11-05 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/004-window-element-filtering/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/commands/plan.md` for the execution workflow.

## Summary

Extend existing list and search commands with --window <handle> parameter to scope element operations to specific windows. Implementation will leverage cross-process automation APIs (AXUIElement on macOS, UI Automation on Windows, AT-SPI on Linux) for element-level access within target windows, with proper permission handling for accessibility services.

## Technical Context

<!--
  ACTION REQUIRED: Replace the content in this section with the technical details
  for the project. The structure here is presented in advisory capacity to guide
  the iteration process.
-->

**Language/Version**: C++17 with platform-specific API bridges (Objective-C++ for macOS)
**Primary Dependencies**:
- macOS: AXUIElement (Accessibility API) for cross-process automation - C++ direct usage with permission handling
- Windows: UI Automation API for element access
- Linux: AT-SPI (Assistive Technology Service Provider Interface)
- Cross-platform: Existing CMake 3.16+ build system, FTXUI v5.0.0 for UI
**Storage**: N/A (runtime window/element enumeration only)
**Testing**: Google Test v1.15.2 for unit and integration testing, platform-specific accessibility API testing
**Target Platform**: Cross-platform desktop (Windows 10+, macOS 10.12+, Linux X11)
**Project Type**: Single executable extending existing window manager
**Performance Goals**: <2 seconds for element enumeration up to 100 elements per window
**Constraints**:
- Must handle accessibility permissions on macOS (System Preferences grant required)
- Backward compatibility with existing list/search commands
- Cross-platform API abstraction without breaking existing functionality
**Scale/Scope**: Single-user desktop automation tool, supporting windows with up to 1000 UI elements

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

**Status**: ✅ PASS - Post-design re-evaluation confirms compliance.

**Initial Evaluation**:
- No established principles in constitution to check against
- Feature extends existing C++17/CMake/FTXUI technology stack
- Maintains backward compatibility as required by constraints
- Uses appropriate platform-specific APIs as recommended in user input
- No architecture violations identified

**Post-Design Re-evaluation** (Phase 1 Complete):
- ✅ **Technology Consistency**: Uses established C++17/CMake/FTXUI stack with platform-specific APIs
- ✅ **Architecture Alignment**: Follows existing pattern of platform-specific implementations under unified interface
- ✅ **API Recommendations**: Implements user-recommended AXUIElement approach for macOS cross-process automation
- ✅ **Performance Targets**: <2 seconds enumeration time aligns with system responsiveness goals
- ✅ **Permission Handling**: Comprehensive permission checking and user guidance implemented
- ✅ **Error Handling**: Robust error handling with platform-specific guidance
- ✅ **Backward Compatibility**: Existing commands unchanged, new functionality is additive via --window parameter

**No constitution violations identified.**

## Project Structure

### Documentation (this feature)

```text
specs/[###-feature]/
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output (/speckit.plan command)
├── data-model.md        # Phase 1 output (/speckit.plan command)
├── quickstart.md        # Phase 1 output (/speckit.plan command)
├── contracts/           # Phase 1 output (/speckit.plan command)
└── tasks.md             # Phase 2 output (/speckit.tasks command - NOT created by /speckit.plan)
```

### Source Code (repository root)
<!--
  ACTION REQUIRED: Replace the placeholder tree below with the concrete layout
  for this feature. Delete unused options and expand the chosen structure with
  real paths (e.g., apps/admin, packages/something). The delivered plan must
  not include Option labels.
-->

```text
src/
├── core/
│   ├── window_manager.cpp
│   ├── window_enumerator.cpp
│   └── element_enumerator.cpp      # New: Element operations interface
├── platform/
│   ├── windows/
│   │   ├── win32_enumerator.cpp
│   │   └── uia_element_finder.cpp  # New: UI Automation implementation
│   ├── macos/
│   │   ├── cocoa_enumerator.cpp
│   │   └── axui_element_finder.cpp # New: AXUIElement implementation
│   └── linux/
│       ├── x11_enumerator.cpp
│       └── atspi_element_finder.cpp # New: AT-SPI implementation
├── cli/
│   ├── list_command.cpp            # Extend: Add --window parameter
│   └── search_command.cpp          # Extend: Add --window parameter
└── ui/
    └── terminal_interface.cpp

tests/
├── unit/
│   ├── core/
│   ├── platform/
│   └── cli/
└── integration/
    └── accessibility_tests.cpp     # New: Cross-platform accessibility testing
```

**Structure Decision**: Single project structure selected as this extends the existing desktop window manager. Platform-specific element finding implementations will be organized under `src/platform/` with OS-specific subdirectories, leveraging the user's macOS recommendations (AXUIElement for cross-process automation).

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| [e.g., 4th project] | [current need] | [why 3 projects insufficient] |
| [e.g., Repository pattern] | [specific problem] | [why direct DB access insufficient] |
