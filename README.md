# Moonmark

![Moonmark approved logo](assets/branding/moonmark-logo.png)

Moonmark is a Windows-first, viewer-first Markdown application with a Rust core and a framework-neutral document and presentation model. Linux is intended, and Android is an eventual target. Moonmark remains file-oriented rather than vault-oriented and contains no browser engine, HTML/DOM application renderer, or local web server.

## Current transition status

The C#/Avalonia/.NET frontend experiment has been discontinued because its runtime and distribution model conflicts with Moonmark's approved runtime policy. It successfully validated substantial renderer, image, window-state, and presentation behavior; it is not considered a technical failure.

The next frontend/rendering implementation has not been selected. The repository therefore temporarily retains the old Avalonia frontend and its Rust `hostfxr` bridge as non-shipping reference material. Do not extend that code. It will be removed after an explicitly approved replacement preserves the necessary behavior.

Rust is the primary language for the shipped application. Python and PowerShell 7 are also approved where appropriate. Any other runtime or implementation language requires specific, explicit user approval; bundling an otherwise unapproved runtime is not an approval loophole.

## Cargo workflow

The intended top-level project interface remains:

```powershell
cargo run
cargo run -- fixtures\moonmark-visual-test.md
cargo check
cargo test
cargo build
cargo build --release
```

The current transitional checkout still invokes the discontinued .NET/Avalonia build from `build.rs`, so these commands presently require the pinned .NET SDK and are not yet representative of the final shipping architecture. This limitation must be removed as part of an eventual user-approved frontend migration, not hidden or worked around by selecting a framework without user approval.

## Architecture

```text
Markdown source
  -> Rust Comrak semantic model
  -> Rust framework-neutral presentation model
  -> replaceable presentation adapter
  -> frontend/rendering implementation (not yet selected)
```

The frontend must not become the semantic document model. Preserve the existing Rust parsing, semantic/presentation models, image policy and cache, syntax highlighting, diagnostics, fixtures, benchmarks, file behavior, and framework-neutral window-state semantics where useful.

The New Moon interface is broad, desktop-first, and achromatic. Syntax highlighting is the only color exception and uses restrained warm/sage tokens with no blue, cyan, or teal.

Relative local images resolve from the Markdown file's directory. Explicit parent-directory, absolute, and `file:///` image references also load automatically after canonicalization; remote images remain disabled. Repeated references to one image share decode/cache work.

Moonmark's normal distribution is intended to be an installer, with an additional portable release using essentially the same application architecture. Package size should reflect useful Moonmark functionality rather than disproportionate language-runtime baggage.

## Fixtures and measurements

The Rust fixture generator and headless renderer benchmark remain useful during the transition:

```powershell
cargo run --bin generate_stress_fixture -- fixtures\generated
cargo run --bin renderer_benchmark -- fixtures\generated\image-stress-250.md
```

The existing GUI smoke commands exercise the discontinued reference frontend and should be treated as behavioral evidence, not as approval to keep that architecture.

See [architecture](docs/ARCHITECTURE.md), [renderer](docs/NATIVE_RENDERER.md), [image pipeline](docs/IMAGE_PIPELINE.md), [window behavior](docs/WINDOW_FRAME.md), [style](docs/UI_STYLE.md), [branding](docs/BRANDING.md), and [building](docs/BUILDING.md).

Major architecture changes require explicit user approval.
