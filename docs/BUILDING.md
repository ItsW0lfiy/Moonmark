# Building

## Toolchain

Tested on Windows x64 with:

- Rust 1.94+ MSVC toolchain
- a C++20-capable MSVC compiler
- Qt 6.11.2 Core, Gui, and Widgets development files
- Windows SDK resource compiler for the executable icon
- PowerShell 7 for the optional bootstrap/package scripts

No .NET SDK/runtime, C#, Avalonia, Node.js, browser engine, CMake, or qmake invocation is part of the normal build. The discovered Qt SDK's qmake executable is queried only as an optional SDK-location fallback.

## Windows setup

Use an existing compatible Qt 6 SDK by setting `MOONMARK_QT_DIR` or `QTDIR`, or download the tested official Qt 6.11.2 MSVC2022 x64 archive into ignored project-local `target/qt-sdk`:

```powershell
pwsh -File scripts/bootstrap_qt.ps1
```

The script verifies the archive against Qt's published SHA-1 file. It does not install Qt globally.

## Cargo workflow

```powershell
cargo fmt --all --check
cargo check
cargo run
cargo run -- fixtures\moonmark-visual-test.md
cargo test
cargo clippy --all-targets --all-features -- -D warnings
cargo build --release
```

`build.rs` discovers Qt, compiles the C++20 adapter with warnings enabled, links Qt dynamically, embeds the Windows icon, and stages Qt DLLs/plugins/assets beside Cargo's executable. Build failure on either language fails Cargo.

Moonmark development milestones use prerelease versions such as `0.1.0-dev.1`, `0.1.0-dev.2`, and `0.1.0-dev.3`. During milestone work, each coherent source, UI, test, or documentation change receives its own descriptive commit before unrelated work begins. Commits are not squashed merely to shorten history, and pushing still requires separate explicit user approval.

## Fixtures and benchmarks

```powershell
cargo run --bin generate_stress_fixture
cargo run --release --bin renderer_benchmark -- fixtures\generated\image-stress.md
cargo run --release --bin renderer_benchmark -- fixtures\generated\large-text.md
```

Generated fixtures are ignored. Native integration smoke tests are part of `cargo test` on Windows.

For a deterministic rendered client-area snapshot during UI review:

```powershell
cargo run -- fixtures\moonmark-visual-test.md --smoke-snapshot
```

The ignored snapshot is written to `target/visual-dev3/moonmark-ui.png`. Run `pwsh -File tools/visual_snapshots.ps1` after a Debug build and fixture generation to capture empty/prose/headings/tables/code/images/narrow/wide cases. Optional process variables `MOONMARK_SNAPSHOT_NAME`, `MOONMARK_SNAPSHOT_WIDTH`, `MOONMARK_SNAPSHOT_HEIGHT`, and `MOONMARK_SNAPSHOT_SCROLL` select the filename, client size, and vertical offset. `MOONMARK_SNAPSHOT_MENU=1` captures the real menu, and `MOONMARK_SNAPSHOT_SELECTION=1` selects the native document before capture. Smoke sessions put QSettings in project-local `target/native-settings`, not the user's application settings. It supplements rather than replaces physical taskbar, Alt+Tab, DPI, and multi-monitor checks.

## Linux

The C++ adapter is mostly cross-platform and the Win32 sections are guarded. On Linux, `build.rs` discovers Qt6Widgets and its transitive Qt modules through `pkg-config`; install a compatible Qt 6 Widgets development package and `pkg-config`. This path is architecturally wired but has not been compiled on a physical Linux host. A Linux milestone must validate compilation/linking, package the platform plugin and system dependencies, and verify AT-SPI/accessibility behavior.
