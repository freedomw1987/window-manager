# Implementation Plan: Window-Specific Element Operations

**Branch**: `004-window-element-filtering` | **Date**: 2025-11-05 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `/specs/004-window-element-filtering/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/commands/plan.md` for the execution workflow.

## Summary

Add `--window <handle>` parameter to existing `list` and `search` commands to enable scoped element discovery and search operations within specific application windows, improving automation precision and reducing false positives from elements in other windows.

## Technical Context

**Language/Version**: C++17
**Primary Dependencies**: FTXUI v5.0.0 (terminal UI), Google Test v1.15.2 (testing)
**Storage**: N/A (runtime window enumeration only)
**Testing**: Google Test with CMake integration (gtest_discover_tests)
**Target Platform**: Cross-platform (Windows Vista+/macOS Sierra+/Linux X11)
**Project Type**: single (command-line tool with optional interactive UI)
**Performance Goals**: < 2 seconds for 100 element enumeration, 50% search speed improvement with window scoping
**Constraints**: Platform APIs (Win32/Core Graphics/X11), < 1 second for window-scoped operations
**Scale/Scope**: Single executable, supports multiple open windows, handles up to 100 elements per window

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

No project constitution file found - using default pass criteria.

## Project Structure

### Documentation (this feature)

```text
specs/004-window-element-filtering/
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output (/speckit.plan command)
├── data-model.md        # Phase 1 output (/speckit.plan command)
├── quickstart.md        # Phase 1 output (/speckit.plan command)
├── contracts/           # Phase 1 output (/speckit.plan command)
└── tasks.md             # Phase 2 output (/speckit.tasks command - NOT created by /speckit.plan)
```

### Source Code (repository root)

```text
# Single project structure (existing)
src/
├── core/               # Window management core
│   ├── window.{cpp,hpp}
│   ├── enumerator.{cpp,hpp}
│   ├── window_manager.{cpp,hpp}
│   └── ...
├── platform/           # Platform-specific implementations
│   ├── windows/
│   ├── macos/
│   └── linux/
├── ui/                 # User interface components
│   ├── cli.{cpp,hpp}
│   └── interactive.{cpp,hpp}
├── filters/            # Search and filtering logic
│   ├── search_query.{cpp,hpp}
│   ├── filter.{cpp,hpp}
│   └── filter_result.{cpp,hpp}
└── main.cpp           # Entry point with argument parsing

tests/
├── unit/              # Unit tests
├── integration/       # Integration tests
└── test_main.cpp      # Test entry point

include/
└── platform_config.h  # Platform detection
```

**Structure Decision**: Using existing single project structure with clear separation of concerns. The current modular design with platform abstraction, core window management, UI layers, and filtering subsystems provides an excellent foundation for adding window-scoped operations without major structural changes.

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

*No violations detected - proceeding with existing architecture.*