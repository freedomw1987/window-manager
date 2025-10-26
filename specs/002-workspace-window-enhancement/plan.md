# Implementation Plan: Workspace Window Enhancement

**Branch**: `002-workspace-window-enhancement` | **Date**: 2025-10-26 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `/specs/002-workspace-window-enhancement/spec.md`

**Additional Requirements**: 輸出結果要增加要包含所有Workspace/ Desktop 中的所有視窗，也要有是否focus 和minimized 的欄位； 用戶不用改變search 的指令，search "keyword" 在程序中支持視窗title和owner 的搜索

## Summary

Enhance the existing C++17 window management tool to enumerate windows across all workspaces/virtual desktops with complete state information (focused, minimized, normal) and extend search functionality to match both window titles and application names transparently. The implementation builds on the current cross-platform architecture using platform-specific APIs (Win32, Core Graphics/Cocoa, X11) while maintaining backward compatibility and existing command-line interface.

## Technical Context

**Language/Version**: C++17 with CMake 3.16+
**Primary Dependencies**: FTXUI for terminal UI, Google Test for testing, platform APIs (Win32/Core Graphics/X11)
**Storage**: N/A (runtime window enumeration only)
**Testing**: Google Test (gtest/gmock) with cross-platform test coverage
**Target Platform**: Cross-platform (Windows, macOS, Linux)
**Project Type**: Single executable (native desktop application)
**Performance Goals**: <3 seconds for window enumeration across all workspaces, <1 second for search operations
**Constraints**: Must maintain existing CLI interface, work within current permission model, support multiple virtual desktops per platform
**Scale/Scope**: Support up to 1000+ windows across unlimited workspaces per platform capabilities

### Platform-Specific Workspace APIs to Research:
- **Windows**: NEEDS CLARIFICATION - Virtual Desktop API integration with existing Win32 enumeration
- **macOS**: NEEDS CLARIFICATION - Mission Control/Spaces API integration with Core Graphics
- **Linux**: NEEDS CLARIFICATION - Extended Window Manager Hints (EWMH) for workspace information

### Current Architecture Extension Points:
- WindowInfo struct needs workspace/state fields
- Platform enumerators need workspace enumeration capabilities
- Search functionality needs dual-field matching (title + owner)
- Output formatters need new field display

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

**Initial Status**: ✅ PASS - No constitution file found, proceeding with established project patterns

**Post-Design Re-evaluation**: ✅ PASS - Design maintains architectural consistency

**Evaluation**:
- ✅ **Architecture Consistency**: Extends existing single-project architecture without introducing new complexity
- ✅ **Technology Stack**: Maintains current C++17/CMake/Google Test technology stack
- ✅ **Interface Compatibility**: Preserves existing CLI interface and backwards compatibility
- ✅ **Dependency Management**: No new dependencies beyond platform APIs (which are already in use)
- ✅ **Design Patterns**: Enhancement follows established cross-platform enumerator pattern
- ✅ **Data Model**: Extends existing WindowInfo structure additively, maintaining backward compatibility
- ✅ **API Design**: New functionality added through enhanced interfaces, existing APIs preserved

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
│   ├── window.hpp/cpp              # Enhanced WindowInfo with workspace/state fields
│   ├── enumerator.hpp/cpp          # Base class with workspace enumeration interface
│   ├── window_manager.hpp/cpp      # Enhanced window management with workspace support
│   └── exceptions.hpp/cpp          # Error handling for workspace operations
├── platform/
│   ├── windows/
│   │   └── win32_enumerator.hpp/cpp    # Windows Virtual Desktop integration
│   ├── macos/
│   │   └── cocoa_enumerator.hpp/cpp    # macOS Spaces/Mission Control integration
│   └── linux/
│       └── x11_enumerator.hpp/cpp      # EWMH workspace support
├── filters/
│   ├── search_query.hpp/cpp        # Enhanced search with title+owner matching
│   ├── filter_result.hpp/cpp       # Results with workspace information
│   └── filter.hpp/cpp              # Filtering logic enhancement
├── ui/
│   ├── cli.hpp/cpp                 # CLI with workspace display options
│   └── interactive.hpp/cpp         # FTXUI enhanced with workspace info
└── main.cpp                        # Entry point with enhanced commands

tests/
├── unit/
│   ├── core/                       # Core class unit tests
│   ├── platform/                   # Platform-specific unit tests
│   └── filters/                    # Search functionality tests
├── integration/
│   ├── workspace_enumeration/      # Cross-workspace enumeration tests
│   ├── search_functionality/       # Title+owner search integration tests
│   └── platform_compatibility/    # Multi-platform workspace tests
└── contract/
    └── api/                        # WindowInfo structure contract tests

include/
└── platform_config.h              # Enhanced platform detection
```

**Structure Decision**: Single project architecture extending the existing pattern. The current structure already supports the cross-platform design needed for workspace enhancement. New workspace and state fields will be added to existing classes, and platform-specific workspace APIs will be integrated into existing enumerators.

## Complexity Tracking

**Status**: ✅ No violations to justify - Design follows established patterns

The enhancement maintains the existing architectural approach without introducing additional complexity:
- Single project structure preserved
- Existing dependency patterns maintained
- No new design patterns introduced
- Additive changes only, no breaking modifications
