# Implementation Plan: Window Focus by Handle

**Branch**: `003-window-focus-by-handle` | **Date**: 2025-11-04 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `/specs/003-window-focus-by-handle/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/commands/plan.md` for the execution workflow.

## Summary

Enable users to focus windows by providing their platform-specific window handle. The system will locate windows across all workspaces, automatically switch to the target workspace if needed, and focus the specified window. This extends the existing C++17 cross-platform window management system with direct window focusing capabilities via handle input.

## Technical Context

**Language/Version**: C++17 with CMake 3.16+
**Primary Dependencies**: FTXUI (terminal UI), Google Test (testing), platform APIs (Win32/Core Graphics/X11)
**Storage**: N/A (runtime window enumeration only)
**Testing**: Google Test framework with unit and integration tests
**Target Platform**: Cross-platform (Windows, macOS, Linux)
**Project Type**: Single native application with cross-platform abstractions
**Performance Goals**: Window focus operations <1s current workspace, <2s cross-workspace switching
**Constraints**: Must handle invalid handles gracefully, workspace switching reliability
**Scale/Scope**: Support existing 50+ window environments, extend current window manager architecture

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

**Status**: NO CONSTITUTION ESTABLISHED
- No project constitution file exists yet at `.specify/memory/constitution.md`
- This feature extends existing C++17 architecture patterns
- Uses established platform abstraction patterns (Win32/Core Graphics/X11)
- Follows existing testing patterns with Google Test
- No architectural violations detected in current technical approach

**Assessment**: PASS - Feature aligns with existing codebase patterns and technical decisions. No constitution violations possible since no constitution exists.

**Post-Design Re-evaluation**: CONFIRMED PASS
- Design maintains existing C++17 architecture patterns
- Uses established platform abstraction layers (Win32/Core Graphics/X11)
- Extends existing WindowManager and CLI interfaces without breaking changes
- Follows existing error handling and testing patterns with Google Test
- No new architectural complexity introduced - builds on existing focus capabilities
- Performance requirements align with existing success criteria patterns

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

```text
src/
├── core/                           # Core window management abstractions
│   ├── window.hpp/cpp             # WindowInfo structure and utilities
│   ├── window_manager.hpp/cpp     # Main facade with existing focus capabilities
│   ├── workspace.hpp/cpp          # WorkspaceInfo and workspace operations
│   ├── enumerator.hpp/cpp         # Abstract platform enumerator interface
│   └── exceptions.hpp/cpp         # Error handling
├── platform/                      # Platform-specific implementations
│   ├── windows/win32_enumerator.hpp/cpp    # Windows focus via Win32 API
│   ├── macos/cocoa_enumerator.hpp/cpp      # macOS focus via Core Graphics
│   └── linux/x11_enumerator.hpp/cpp       # Linux focus via X11
├── ui/                            # User interface components
│   ├── cli.hpp/cpp                # Command-line interface with focus commands
│   └── interactive.hpp/cpp        # FTXUI-based interactive interface
├── filters/                       # Window filtering and search
└── main.cpp                       # Application entry point

tests/
├── unit/                          # Unit tests for individual components
├── integration/                   # Cross-platform integration tests
└── test_main.cpp                  # Test runner
```

**Structure Decision**: Single project structure with existing cross-platform abstraction. New window focus functionality will extend existing WindowManager and platform enumerator interfaces rather than creating new architectural components.

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| [e.g., 4th project] | [current need] | [why 3 projects insufficient] |
| [e.g., Repository pattern] | [specific problem] | [why direct DB access insufficient] |
