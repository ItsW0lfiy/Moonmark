# Moonmark 0.1.0-dev.6 validation

Validation date: 2026-09-11. Platform: Windows x64. Starting local and remote HEAD: `81796b372d25ae51a722e7d7957e7f84e059d220`.

## Product behavior

- One process retains one Rust backend, native `DocumentView`/`QTextDocument`, outline, watcher, debounce timer, zoom, scroll, selection, and image state per open document.
- Opening a canonical duplicate activates its existing session. Closing the final session returns to the empty state. No single-instance lock or inter-process routing was added, so independent Moonmark processes remain supported.
- `.txt` files bypass Comrak, headings, links, images, and the outline. The literal fixture preserves Markdown-looking characters, line breaks, tabs, Unicode, spaces, selection, and long horizontal lines.
- An inactive plain-text session's watcher reloaded that session once while the active Markdown session's load, parse, and image-request counters remained unchanged. The plain-text parse delta remained zero.
- Heading navigation resolves current named-anchor geometry, leaves top breathing room, and uses interruptible cubic-out scrollbar animation. Mouse-wheel notches use a short retargetable animation; pixel/touchpad scrolling and scrollbar dragging remain direct. Keyboard/mouse input cancels owned scroll motion.
- Sidebar width uses a short native animation. `MOONMARK_REDUCED_MOTION=1` makes owned transitions immediate for accessibility and deterministic testing.

## Renderer review

The code-block review found one continuous low-contrast graphite frame with an integrated neutral language header, real separator, quiet Copy action, syntax color, preserved blank lines, and no hard outer stroke. Qt's native `QTextFrameFormat` has no corner-radius property, so Moonmark keeps selectable native text instead of rasterizing the block to fake rounded corners.

Tables retain immediate column tracking through faint vertical separators, stronger header/horizontal structure, minimal outer framing, compact numeric columns, native selection, and inline-code formatting. They do not read as spreadsheet controls.

The prose inline-code screenshot exposed a one-pixel lower/right edge caused by extending `QTextLayout` ink geometry vertically in the decorative padding pass. The fix leaves Qt's native character background vertically authoritative and paints only horizontal rounded breathing room. It changes neither text, copy output, wrapping, selection, nor accessibility. Wrapped prose, lists, quotes, and table cells were rechecked after the fix.

The existing image-layout regression remains fixed. Loaded images and placeholders report zero unexplained excess block height after declared margins across reload, narrow/wide resize, and 80/100/125/150% zoom.

## Performance measurements

Release measurements are local samples, not hard CI thresholds.

| Case | Result |
| --- | --- |
| Retained two-document switch, 40 alternations | 323 microseconds average |
| Heading animation setup | 2,501 microseconds |
| Sidebar animation setup | 255 microseconds |
| 650,000-character literal text construction, Release | 485,811 microseconds; parse count 0 |
| Image stress semantic parse, 20 runs | p50 0.680 ms; p95 0.877 ms |
| Image stress presentation construction | p50 22.078 ms; p95 25.168 ms |
| 10,000-block semantic parse, 20 runs | p50 55.778 ms; p95 66.971 ms |
| 10,000-block presentation construction | p50 15.134 ms; p95 22.018 ms |
| Image stress completion | 250 loaded, 5 intentional missing, 0 decode failures, 6 canonical requests, 15.36 MB cache, 369 ms completion |
| One moderate document after 3 s | 120.35 MiB working set; 45.99 MiB private |
| Four moderate retained documents after 3 s | 122.12 MiB working set; 48.05 MiB private |
| Four documents including large text and image stress | 196.66 MiB working set; 126.14 MiB private |

Normal-document zoom action/paint times remained in the low tens of milliseconds. The 10,000-block fixture required approximately 110–118 ms of synchronous format work and approximately 1.0–1.18 seconds total including Qt relayout and exhaustive verification. All zoom cases reported parse, file-load, document-construction, and image-request deltas of zero. This large-document relayout is the main known performance limitation.

## Native and package checks

The complete Cargo test run passed 21 core unit tests, 4 acceptance tests, 2 native frontend tests, and documentation tests. Strict Clippy passed with `-D warnings`. The C++20 `/W4` build emitted no compiler warning. Release build and portable assembly passed.

The packaged application was tested with `PATH` limited to `C:\Windows\System32;C:\Windows` and Qt discovery variables removed. Icon, render/selection/code-copy, style, retained multi-document switching, layout-transition counters, and 250-image stress smokes passed.

The portable output contains 16 files:

- `Moonmark.exe`: 6,388,736 bytes (6.09 MiB)
- complete folder: 35,021,676 bytes (33.40 MiB)
- dependency/assets/legal overhead: 28,632,940 bytes (27.31 MiB)
- compressed ZIP: 15,530,358 bytes (14.81 MiB)

The largest app-local dependencies are Qt6Core (10,363,704 bytes), Qt6Gui (9,546,552), Qt6Widgets (6,497,080), qwindows (991,032), and the app-local MSVC runtime set (1,048,216 combined). `dumpbin /DEPENDENTS` found Qt, MSVC CRT, and Windows system libraries; system ICU is supplied by Windows. No CLR/hostfxr, JVM, Node.js, browser engine, WebView2, Chromium, or Qt WebEngine dependency is present.

## Visual artifacts inspected

Ignored snapshots under `target/visual-dev6` cover: empty, prose, headings/lists, tables, code, inline prose/table, inline wrap, inline selection, plain text, image gap, image stress, narrow, wide, concept, sidebar hidden, menu, whole-document selection, zoom 80/100/125/150, code at 150%, and a Markdown-plus-text multi-document window.

Inspection found an achromatic broad document surface, clear active/inactive file entries, literal text presentation, integrated code metadata, quieter code/table framing, retained neutral selection, and no theme blue. The user-reported prose inline-code edge was visible before the geometry correction and absent afterward.

## Commands

```powershell
cargo fmt --all --check
cargo check
cargo test
cargo clippy --all-targets --all-features -- -D warnings
cargo build --release
cargo run --release --bin renderer_benchmark -- fixtures/generated/image-stress.md
cargo run --release --bin renderer_benchmark -- fixtures/generated/large-text.md
pwsh -File tools/render_performance.ps1 -Executable target/release/moonmark.exe
pwsh -File tools/visual_snapshots.ps1 -Executable target/debug/moonmark.exe
pwsh -File scripts/package_windows.ps1
git diff --check
```

## Manual validation still required

Automated Qt/Win32 checks do not replace physical Windows 10/11 validation of Snap, Alt+Space details, taskbar work area, mixed-DPI multi-monitor transitions, real high-resolution touchpad input, screen-reader/UI Automation output, or prolonged animation frame pacing. Linux remains architecturally wired through guarded platform code and `pkg-config`, but was not compiled on this Windows host. Android, editor, installer, updater, signing, and WinGet implementation were not started.
