# AGENTS.md — Moonmark Project Rules

## Purpose

This file contains hard project rules for any Codex agent or other coding agent working on Moonmark.

These rules exist to prevent unapproved architecture drift, language changes, browser-runtime dependencies, unwanted UI design changes, broken Windows behavior, and other major decisions being made without explicit user approval.

Read this file before modifying Moonmark.

If a task conflicts with this file, stop and ask the user before making the conflicting change.

---

# 1. User authority

Moonmark belongs to the user.

Do not make major project decisions merely because another approach is easier, more familiar, more fashionable, or preferred by the agent.

The user must explicitly approve major changes before they are made.

This includes changes to:

- primary programming language
- GUI framework
- rendering architecture
- build system
- runtime
- storage architecture
- application model
- major dependency choices
- native versus web architecture
- supported platform strategy

If a requested task would require one of these changes and approval has not already been given:

**STOP AND ASK FIRST.**

Do not implement the change and explain afterward.

---

# 1A. Approval semantics

Discussion is not approval. Investigation is not approval. A syntax preview is not approval. A benchmark is not approval. A prototype is not approval unless the user explicitly authorized implementing that prototype. Past use of a technology is not current approval, and an AI suggesting a technology is not approval.

Only explicit user approval permits introducing a currently closed language, runtime, framework, or architecture.

---

# 2. Runtime and execution policy

Programs should not rely on a runtime when that runtime dependency is reasonably avoidable.

The only generally approved runtime and execution ecosystems are:

- Rust
- Python
- PowerShell 7

Any other runtime requires **specific, explicit user approval** before it may be introduced. This includes, but is not limited to:

- .NET / CLR
- JVM / Java runtime
- Node.js as an application runtime
- other managed or general-purpose language runtimes

Bundling an otherwise unapproved runtime with Moonmark does not make that runtime approved. Requiring users to install .NET is not approved, and bundling a private .NET runtime is also not approved unless the user explicitly authorizes it.

The following are not valid reasons to bypass this policy:

- framework convenience
- easier implementation
- cross-platform convenience
- portability claims
- implementation difficulty
- an agent preferring a technology
- an existing framework requiring the runtime
- a dependency already being installed on the developer's machine

If Moonmark appears to require an unapproved runtime, stop and request explicit user approval instead of introducing it.

Dependencies are not banned. Native libraries, rendering libraries, image libraries, codecs, parsers, system libraries, Rust crates, and other justified dependencies remain allowed. This policy concerns avoidable runtime dependence, not dependency count.

---

# 3. Approved languages

The approved Moonmark implementation languages are:

- Rust
- C++

The approved Moonmark support and tooling languages are:

- Python
- PowerShell 7 where appropriate

Rust and C++ are both first-class implementation languages for Moonmark. Use whichever provides
the clearer, safer, and more maintainable implementation for a subsystem. There is no required
language percentage or fixed subsystem boundary, and working code must not be rewritten merely to
change the Rust/C++ ratio. This approval is specific to Moonmark and does not approve C++ for the
user's other projects.

Python is fully approved for application programs as well as development tooling, test utilities, maintenance scripts, asset processing, and build/support tasks. Do not describe Python as tooling-only.

PowerShell 7 is approved where it makes technical sense.

C# is closed. It is not a first-class Moonmark language, frontend language, fallback, or acceptable application/UI language. Do not introduce or expand C#.

C++20, Qt 6 Widgets, and QTextDocument are explicitly approved for Moonmark's Windows/Linux
desktop application. Qt Quick, QML, Qt WebEngine, and browser-backed Qt components are not
approved. CXX-Qt is not automatically approved; use it only after a separate technical evaluation
and explicit user approval.

Other implementation languages are not approved unless the user specifically and explicitly approves them.

Shared code should avoid unnecessary assumptions such as Windows drive letters, backslash-only paths, Win32-only document concepts, Windows-only behavior in platform-neutral models, and permanent ordinary-filesystem-path assumptions. Keep platform-specific behavior behind deliberate boundaries without prematurely building a giant abstraction layer.

---

# 4. Cargo is the primary project interface

Moonmark is Cargo-driven.

The normal top-level development commands must remain:

    cargo run
    cargo check
    cargo build
    cargo build --release
    cargo test

Cargo is the normal builder, verifier, test entry point, and runner for the complete project.

`cargo run` must launch Moonmark from the project root.

Rust is responsible for the shipped application architecture and remains the launcher/entry point.

Do not replace Cargo with CMake, MSBuild, `dotnet`, Ninja, qmake, npm, pnpm, yarn, or another primary developer workflow.

Supporting tooling may use Python or PowerShell 7 where useful.

---

# 4A. Compact and portable distribution

Moonmark should remain reasonably compact and portable. Do not impose an arbitrary hard package-size limit.

Package size is justified when the bytes provide Moonmark functionality such as rendering, image decoding, text shaping, fonts, UI implementation, syntax highlighting, codecs, or operating-system integration. The unacceptable case is disproportionate runtime baggage that primarily supports an implementation language or runtime rather than Moonmark itself.

The executable and package do not need to be artificially tiny. They must avoid being unnecessarily huge.

Moonmark's primary normal distribution will be an installer. It may install Moonmark into an appropriate program directory and provide Start Menu integration, shortcuts, file associations, uninstall support, and future update integration if separately approved. It should install Moonmark itself, not primarily bootstrap an otherwise avoidable language runtime.

Moonmark will also provide a portable release. It should run directly from its extracted directory, use essentially the same application architecture as the installed edition, require no installation, and avoid unrelated machine-wide dependencies where reasonably possible. Do not create separate unrelated installed and portable implementations.

---

# 5. Approved Qt desktop frontend and discontinued Avalonia frontend

The C#/Avalonia/.NET direction is discontinued. It is not Moonmark's approved shipping architecture.

Do not authorize or expand:

- C# application or frontend ownership
- Avalonia as Moonmark's selected frontend
- .NET / CLR as a Moonmark runtime
- `hostfxr` or CLR hosting as Moonmark architecture
- a privately bundled .NET runtime

The Avalonia experiment successfully validated substantial renderer behavior and the framework-neutral architecture. It is being discontinued because its runtime and distribution model conflicts with Moonmark's runtime policy, not because the experiment was a technical failure.

Measured Windows x64 results that informed this decision were approximately:

- Moonmark-owned binaries: 6.07 MiB
- framework-dependent package: 32.72 MiB, requiring installed .NET
- self-contained non-AOT package: 109.21 MiB
- bundled .NET runtime portion: 76.46 MiB

The C#/Avalonia implementation was obsolete migration/reference material. Its useful behavioral knowledge was preserved in the native renderer, tests, fixtures, and architecture history before its source and runtime integration were removed. Do not restore it. In particular:

- do not add new C# features
- do not expand Avalonia
- do not refactor C# merely to improve an architecture that will not ship
- do not make new Moonmark functionality depend on C#
- keep C#/Avalonia/.NET absent from the shipping architecture

The approved desktop frontend is C++20 with Qt 6 Widgets, using QTextDocument and native Qt
document APIs for Markdown presentation. Windows is the primary target and Linux is the secondary
desktop target. Cargo remains the top-level workflow and launches the single native Moonmark
process. Rust and C++ subsystem ownership follows the best-tool-wins rule in section 3.

CXX-Qt is not part of this approval. A narrow C ABI, `cxx`, generated bindings, or another native
bridge may be selected on technical merit, but the bridge must have explicit ownership, a small
unsafe surface, clear errors, and no second process or localhost IPC.

---

# 5A. Framework-neutral presentation boundary

Moonmark must own its document and presentation concepts independently of any frontend framework.

The preferred conceptual flow is:

    Markdown source
        ->
    Moonmark document/core logic
        ->
    Moonmark semantic document model
        ->
    Moonmark framework-neutral presentation model
        ->
    replaceable presentation adapter
        ->
    Qt Widgets / QTextDocument presentation adapter
        ->
    native display

Moonmark-owned concepts may include, as appropriate:

- document
- heading
- paragraph
- text span
- inline code
- code block
- blockquote
- list
- list item
- task item
- table
- image
- link
- horizontal rule
- table-of-contents / outline entry

These concepts must not be defined in terms of frontend controls or framework objects. Framework-specific types must remain inside the eventual presentation adapter.

Important document/core tests should be able to run headlessly without creating a frontend application, window, GPU context, or UI thread wherever the tested behavior does not inherently require UI.

The Moonmark presentation model should be testable separately from its eventual rendering adapter.

## Moonmark-owned document view

Prefer a Moonmark-owned document presentation surface rather than delegating Moonmark's renderer identity to a monolithic third-party Markdown control.

The document surface should remain conceptually replaceable so a future presentation implementation can adopt custom layout, virtualization, retained drawing, or another optimized approach if large-document measurements justify it.

Do not prematurely build a custom GUI toolkit, text engine, font shaper, window system, or accessibility stack merely for independence.

Moonmark should own Moonmark-specific rendering behavior while the eventual approved frontend handles generic UI plumbing.

## Theme and design tokens

Keep important Moonmark visual semantics coherent and reusable rather than scattering arbitrary framework constants throughout the codebase.

Prefer Moonmark-owned design tokens or clearly centralized theme definitions for concepts such as:

- application surfaces
- document surfaces
- elevated surfaces
- borders
- primary/secondary text
- hover/pressed/focus state
- spacing
- corner radii
- typography

The current Qt frontend implements those tokens, while the Moonmark design language remains understandable independently of individual controls.

# 6. Absolute browser-runtime prohibition

Moonmark's desktop implementation must remain a real native desktop application.

Do not introduce or require:

- Microsoft Edge
- Microsoft Edge WebView2
- Tauri
- Electron
- Chromium
- bundled Chromium
- CEF
- Qt WebEngine
- WebKit
- Edge-based shells
- PWAs
- browser-wrapper desktop technology
- HTML/CSS/JavaScript desktop frontends
- Svelte
- React
- Vue
- Vite runtime
- local HTTP UI servers
- localhost frontend servers
- DOM-based application rendering

Do not use a browser engine to render Moonmark.

Do not introduce a hidden browser dependency.

Do not describe WebView2 as an acceptable implementation detail.

It is not acceptable in this project.

---

# 7. No unapproved technology substitutions

Do not silently replace an approved project-level technology or architecture with another.

Examples of forbidden behavior include:

- Cargo -> CMake/MSBuild as the primary workflow
- Rust/C++ -> C# or another unapproved implementation language
- an approved runtime -> .NET, JVM, Node.js, or another unapproved runtime
- native UI -> HTML/browser frontend
- Comrak -> unrelated parser without reason
- no selected frontend -> an agent-selected frontend

However, if a change alters the project-level architecture, runtime model, primary workflow, approved language set, or major framework choice:

1. explain the actual problem,
2. explain the alternatives,
3. explain the tradeoffs,
4. make a recommendation,
5. wait for user approval.

Do not make the migration first.

---

# 8. Markdown architecture

Moonmark is a viewer-first Markdown application.

Current preferred parser:

- Comrak

Preserve the useful existing Rust parsing and document-processing work.

The current architecture direction is conceptually:

    Markdown source
        ->
    Rust / Comrak parsing
        ->
    Moonmark semantic document model
        ->
    Moonmark framework-neutral presentation model
        ->
    replaceable presentation adapter
        ->
    Qt Widgets / QTextDocument presentation adapter
        ->
    native display

The framework-neutral semantic and presentation boundaries are intentional architecture. Do not bypass them by making a frontend framework's controls the document model.

Do not convert the application into an HTML-rendering pipeline.

Qt 6 Widgets and QTextDocument are the approved desktop presentation technology. Construct the
document through native Qt APIs; do not turn Markdown into HTML and feed HTML into QTextDocument.
CXX-Qt remains unapproved, and C#/Avalonia/.NET and browser rendering remain prohibited.

Preserve useful Rust-side work including Comrak parsing, semantic and presentation models, image policy and caching, syntax-highlighting policy, diagnostics, benchmarks, fixtures, stress tests, file behavior, window-state semantics, performance work, and renderer expectations.

# 8A. Documents remain ordinary external files

Moonmark is file-oriented, not vault-oriented.

A Markdown document opened in Moonmark must remain an ordinary external file owned by the user and the filesystem.

Moonmark may **use a file; it must not hug the file.**

Normal viewing must not require:

- importing the document into Moonmark
- copying the document into a Moonmark-managed location
- converting it into a proprietary format
- creating a vault
- creating a workspace
- claiming ownership of the surrounding folder
- keeping an unnecessary exclusive file handle open

When practical, viewer mode should read the file and release the operating-system handle rather than holding the file open indefinitely.

Other applications must remain able to:

- read the same Markdown file
- modify the same Markdown file
- rename or otherwise manage files subject to normal operating-system rules

Moonmark may watch an opened document for external changes and offer or perform sensible refresh behavior.

Relative images and other relative resources should resolve from the document's source context without Moonmark claiming ownership of that directory.

Explicitly referenced local images must support valid paths outside the Markdown document's own directory after appropriate canonicalization, including `../` parent traversal, absolute local paths, and `file:///` paths. Do not restore the rejected folder-approval model that automatically blocks such references merely because they are outside the document directory.

Remote HTTP/HTTPS image behavior remains governed by Moonmark's existing project policy. Do not make network requests merely because a local document was opened.

Future editing support must preserve this philosophy.

Editing must not justify permanently write-locking a document.

Prefer safe save behavior. When Moonmark eventually has unsaved edits and the on-disk file changes externally, detect the conflict rather than silently overwriting one side.

Do not turn Moonmark into a vault/database application unless the user explicitly changes this rule.

---

# 9. Markdown rendering quality is a priority

Moonmark's renderer must not be treated as an afterthought.

Support and render well:

- paragraphs
- H1-H6
- bold
- italic
- bold + italic
- strikethrough
- inline code
- fenced code blocks
- blockquotes
- unordered lists
- ordered lists
- nested lists
- task lists
- horizontal rules
- links
- autolinks
- images
- local relative images
- local absolute images
- tables
- GFM tables
- escaped Markdown
- correct line-break behavior

Do not flatten document structure into generic paragraphs.

Heading hierarchy, spacing, lists, tables, code, blockquotes, and images must look deliberate.

---

# 10. FeatherMD layout reference

The user liked the broad rendered-document presentation of FeatherMD more than Moonmark's earlier compact centered layout.

Use FeatherMD only as an experience/layout reference.

Do not copy its source code.

Moonmark must NOT use a narrow blog/article-style center column.

Do not squeeze the document into roughly 700-900 pixels while leaving huge unused areas on both sides.

Moonmark's **desktop presentation** is a broad desktop Markdown viewer.

The desktop presentation should use the available desktop width.

Default document behavior should have:

- broad usable content area
- sensible side padding
- room for large images
- room for tables
- room for code
- room for screenshots and diagrams

Large windows should visibly benefit from their available width.

The default presentation should feel closer to a proper desktop document viewer than a website article.

---


# 10A. ChatGPT / Discord-inspired Markdown presentation

The user also likes the general Markdown presentation style commonly seen in ChatGPT and Discord.

Use those applications as **visual and interaction references only**.

Do NOT copy:

- their exact layouts
- their narrow chat-column widths
- proprietary assets
- browser/web technology
- CSS
- DOM rendering
- Discord branding
- ChatGPT branding

The goal is:

**FeatherMD-like desktop spaciousness + ChatGPT/Discord-like Markdown styling + Moonmark's own monochrome lunar identity.**

The important distinction is:

**Use ChatGPT/Discord as styling inspiration, NOT as width/layout inspiration.**

Moonmark's desktop presentation must remain a broad desktop Markdown viewer.

Do not turn the desktop document presentation into a narrow centered chat conversation.

## Normal body text

Body text should feel:

- clean
- readable
- modern
- conversational
- slightly compact without being cramped
- less like a printed book/article
- more like polished application Markdown

Avoid oversized article typography.

Avoid excessive paragraph gaps.

Keep line height comfortable.

## Headings

Headings should have clear hierarchy but should not become enormous newspaper-style headings.

Aim for:

- obvious H1/H2/H3 distinction
- strong weight hierarchy
- deliberate spacing
- compact enough to feel application-native

Do not make every heading nearly the same size.

Do not make H1 consume absurd vertical space.

## Bold and italic

Bold should be clearly heavier while remaining visually restrained.

Italic should be subtle and readable.

Nested combinations such as bold+italic must remain visually coherent.

## Inline code

Inline code should feel closer to ChatGPT/Discord Markdown than plain unstyled monospace text.

Prefer:

- monospace font
- subtle graphite background
- small horizontal/vertical padding
- mild corner radius if technically practical
- neutral grayscale foreground
- no blue tint

Do not create oversized pill buttons.

Inline code must remain part of the text flow.

## Code blocks

Code blocks should use a clean dark application style inspired by ChatGPT/Discord code presentation.

Requirements:

- dark graphite/near-black surface
- readable monospace font
- comfortable internal padding
- preserved indentation and whitespace
- horizontal overflow support where appropriate
- subtle separation from surrounding text
- restrained corner radius if used
- no bright colored border
- no blue accents

Do not turn every code block into a giant card.

Syntax highlighting may be added later, but layout quality comes first.

Any future syntax-highlighting palette must respect Moonmark's no-blue rule unless the user explicitly approves individual colors.

## Blockquotes

Blockquotes should use a restrained style similar to Discord/ChatGPT Markdown.

Prefer:

- narrow neutral gray/silver vertical rule
- modest indentation
- slightly muted text
- little or no background fill

Do not use:

- blue quote bars
- bright colored backgrounds
- giant rounded cards
- neon accents

## Lists

Lists should be compact, clean, and easy to scan.

Use:

- sensible bullet/number spacing
- restrained indentation
- clear nested hierarchy
- comfortable list-item spacing

Avoid overly large nested indentation that wastes horizontal space.

Nested lists should remain readable in wide documents.

## Task lists

Task lists should render as clear native-style checkboxes rather than leaving raw `[ ]` / `[x]` text visible when practical.

Checkboxes must use the Moonmark achromatic palette.

No blue checked-state fill.

## Links

Links should feel clearly interactive while remaining monochrome.

Use combinations such as:

- underline
- moon-silver text
- brighter neutral gray
- font weight
- hover underline changes
- subtle luminance shifts

Do NOT use browser-default blue.

Do NOT use cyan, teal, indigo, or blue-gray.

## Tables

Tables should be visually clean and application-like.

Prefer:

- subtle graphite separators
- modest cell padding
- slightly stronger header row
- minimal boxiness
- use of available document width

Do not make tables look like giant spreadsheet widgets unless the content genuinely requires that.

## Horizontal rules

Horizontal rules should be thin, understated neutral dividers.

Use grayscale only.

## Images

Images should integrate naturally into the document flow.

Prefer:

- large readable presentation where space allows
- proportional scaling
- sensible spacing above/below
- broad use of available viewport width
- no tiny blog-thumbnail default

Do not constrain images just because normal prose has margins.

Wide screenshots and diagrams should be allowed to benefit from Moonmark's broad layout.

## Paragraph spacing

Paragraph spacing should feel closer to conversational Markdown than traditional publishing.

Use enough spacing for clear separation without giant gaps.

Avoid both:

- dense wall-of-text compaction
- excessive empty vertical space

## Selection and focus

Text selection, keyboard focus, interactive hover, and active states must remain visible but achromatic.

Use:

- silver
- white
- neutral gray
- graphite

Never allow system or UI-framework default blue to leak into Moonmark-controlled surfaces.

## Design identity boundary

The rendered Markdown must still look like Moonmark.

Do not make it visually identical to Discord or ChatGPT.

The inspiration should influence:

- spacing
- hierarchy
- code treatment
- quote treatment
- list compactness
- modern application readability

Moonmark's **desktop presentation identity** remains:

- monochrome
- black lunar
- native desktop
- broad document canvas
- viewer-first
- no blue

These desktop-layout rules do not require the future Android UI to imitate a desktop window.

---


# 11. Image-heavy performance

Image-heavy Markdown performance is an important reason Moonmark exists.

Do not:

- reread every image during normal resizing
- re-decode every image on every layout change
- reparse Markdown because the window dimensions changed
- spawn one thread per image
- keep unlimited full-resolution decoded images forever

Use:

- bounded concurrency
- bounded caching
- sensible image scaling
- background work where practical
- UI-thread-safe result delivery

Prefer Rust to own image pipeline policy and bookkeeping.

Large documents must remain responsive.

---

# 12. Resize behavior

Window resizing must primarily cause layout reflow.

It must not trigger expensive semantic rebuilds.

Normal resize, maximize, restore, fullscreen, and monitor movement must not unnecessarily:

- reopen the Markdown file
- reparse Markdown
- rebuild the semantic document
- reread all images
- redecode all images
- discard useful caches

---

# 13. Window state model

Moonmark must distinguish normal maximize from F11 fullscreen.

These are separate behaviors.

Conceptually the application has:

- Normal
- Maximized
- BorderlessFullscreen

Do not implement maximize and fullscreen using one shared ambiguous boolean.

Preserve pre-fullscreen window state and placement.

Prefer Rust to own this state model.

---

# 14. Maximize behavior

The maximize button performs normal Windows maximize.

When maximized:

- Moonmark's custom top bar remains visible
- the Windows taskbar remains visible
- the window remains normally managed by Windows
- the maximize control becomes restore
- restore returns to the prior normal window placement

The maximize button must NOT activate F11 fullscreen.

F11 must NOT replace or overwrite maximize behavior.

---

# 15. F11 behavior

F11 toggles **windowed borderless fullscreen**.

It is not normal maximize.

It is not exclusive display fullscreen.

When entering F11:

- preserve the previous normal/maximized state
- use the monitor containing Moonmark
- use the same Moonmark window
- hide Moonmark's custom title bar
- remove visible window borders/chrome
- cover the current monitor
- do not change display resolution
- do not change refresh rate
- do not recreate the document unnecessarily
- do not reparse Markdown
- do not reload all images

Pressing F11 again exits borderless fullscreen.

Escape also exits borderless fullscreen while fullscreen is active.

When leaving fullscreen:

- previous Normal returns to Normal with previous placement
- previous Maximized returns to Maximized

Do not corrupt maximize state.

---

# 16. Windows window behavior

The custom Moonmark titlebar must still behave like a proper Windows desktop window.

Required behavior includes:

- draggable titlebar
- minimize
- maximize/restore
- close
- double-click titlebar maximize/restore
- normal resize borders
- Windows snapping
- multi-monitor behavior
- DPI scaling
- taskbar presence
- proper focus behavior
- Alt+Space system menu where practical

Windows 11 Snap Layout support should be retained where reasonably possible.

Investigate proper native hit testing instead of faking it with fragile geometry hacks.

Use an approved language for Windows integration. Keep Windows-specific behavior behind clear platform boundaries where practical.

---

# 17. Desktop UI design direction

The user dislikes the previous Moonmark UI.

Do not preserve a bad UI merely because it already exists.

Moonmark's desktop UI should be:

- black
- lunar
- native
- wide
- clean
- restrained
- desktop-first
- viewer-first
- responsive
- low-clutter

Avoid:

- mobile-style layouts
- giant rounded cards
- SaaS dashboard aesthetics
- web-template aesthetics
- excessive center compaction
- giant navigation surfaces
- unnecessary decoration
- starfield/galaxy gimmicks

The Markdown document should remain the dominant surface.

---

# 18. ABSOLUTELY NO BLUE

This is a hard visual requirement.

The user does not want blue in Moonmark.

Do not use:

- blue
- cyan
- teal
- navy
- azure
- indigo
- blue-gray
- blue-tinted silver

Do not add blue because "moonlight is blue."

Do not add blue because Windows or a UI framework uses blue by default.

The default Moonmark palette must be achromatic.

Use:

- black
- near-black
- charcoal
- graphite
- neutral gray
- moon-silver
- off-white
- white

Where practical, neutral grayscale colors should use equal RGB channel values.

Blue must not appear in:

- titlebar
- buttons
- hover states
- pressed states
- links
- selection
- focus indicators
- scrollbars
- tables
- code blocks
- blockquotes
- settings
- active items
- placeholders
- progress indicators

If Windows or the selected UI framework defaults to blue inside a Moonmark-controlled surface, override it with a neutral theme value.

Do not remove accessibility indicators merely to avoid blue.

Use silver/gray/white indicators instead.

---

# 19. Links are not blue

Markdown links must not use default browser blue.

Use combinations such as:

- underline
- neutral moon-silver
- brighter grayscale text
- weight
- hover underline changes

Links must remain recognizable without blue.

---

# 20. Black lunar does not mean blue lunar

Moonmark's visual language is:

- new moon
- shadow
- graphite
- dark stone
- black
- gray
- silver
- moon surface

It is NOT:

- blue night
- cyan glow
- purple galaxy
- holographic sci-fi
- starfield UI

Keep the lunar identity monochrome and restrained.

---

# 21. Native errors only

Moonmark must never display browser-style failure pages such as:

    couldn't load server

Moonmark does not use a web server.

Errors must be presented through native Moonmark UI.

Missing images or failed files must not destroy the rest of the document.

---

# 22. UI changes require visual judgment

Do not treat "the program compiles" as sufficient for UI work.

When changing rendering or window presentation, inspect the actual result where tooling permits.

Check:

- document width
- typography
- headings
- links
- images
- lists
- tables
- code blocks
- blockquotes
- scrollbars
- focus states
- selection
- titlebar
- fullscreen
- maximize

Do not claim visual requirements are complete without checking them.

---

# 23. Performance testing

For relevant changes, test with:

- small Markdown files
- huge text-only files
- image-heavy documents
- many local images
- large screenshots
- nested lists
- wide tables
- repeated resizing
- repeated maximize/restore
- repeated F11 enter/exit

Do not hide known performance regressions.

---

# 24. Rust safety

Keep unsafe Rust narrow.

Unsafe code is acceptable where native FFI genuinely requires it.

Every manually written unsafe block should:

- be minimal
- have a clear safety invariant
- be documented

Do not use unsafe merely to bypass Rust's ownership model.

---

# 25. Dependencies

Do not add dependencies casually.

Before adding one, consider:

- whether it is necessary
- whether the standard libraries or current approved dependencies already solve it
- whether it is actively maintained
- whether it introduces a large runtime
- whether it introduces any browser/web technology
- whether it couples framework-specific behavior into the Moonmark core
- whether its benefit justifies its cost

## Zero-paid / zero-ads dependency rule

Moonmark must be buildable and fully usable without requiring the user or developer to pay for required functionality.

Moonmark itself must not display advertisements, include an advertising SDK, require an end-user subscription, or place normal application functionality behind a paid subscription.

Required Moonmark functionality must not depend on:

- paid libraries or controls
- Pro/Premium-only components
- subscriptions
- recurring developer fees
- per-seat licenses
- runtime license keys for paid functionality
- ad-supported SDKs that require Moonmark to display advertising
- dependencies that require advertisements in Moonmark
- a "free" tier that withholds functionality Moonmark actually requires behind payment

Prefer free/open-source dependencies with licenses suitable for Moonmark.

A project having an optional commercial tier is not automatically disqualifying, but Moonmark must not depend on that paid tier for required behavior.

If a dependency changes licensing or moves a required feature behind payment, do not silently accept the new requirement. Stop, report the impact, and propose free/open-source replacements or an in-house Moonmark implementation where practical.

Avalonia and Avalonia Pro are outside the approved architecture. Paid, Pro, subscription-gated, license-key-gated, and ad-supported required components remain prohibited regardless of any future frontend decision.

Do not add Node.js tooling.

Do not add npm.

Do not add a JavaScript build pipeline.

---

# 26. Do not overengineer

Moonmark is a Markdown viewer.

Do not invent unnecessary architecture such as:

- plugin framework
- scripting VM
- database
- dependency-injection framework
- microservices
- localhost daemon
- custom browser engine
- ECS architecture

unless the user explicitly approves a future feature requiring it.

Keep the application understandable.

---

# 27. Settings ownership

Prefer one authoritative, framework-neutral settings model rather than duplicated frontend and core copies.

Design code so future settings can reasonably include:

- font size
- line spacing
- document padding
- preferred content width
- image behavior
- theme options
- fullscreen preferences

Keep important settings understandable and avoid burying them behind an unnecessary framework boundary.

# 27A. Platform roadmap

Moonmark is **Windows-first**, not Windows-only.

Current priority:

1. Windows desktop
2. Linux desktop
3. Android / phone

Windows is the primary development and release target today.

Linux is an intended future supported desktop platform.

Android is an intended future supported mobile platform, including eventual APK distribution.

Windows may receive features and updates before Linux or Android. The existence of future platform targets does not require simultaneous releases.

Shared Moonmark behavior should remain portable across the intended platforms where practical and remain within the approved language/runtime policy.

Portable/shared behavior should include, where reasonable:

- Markdown parsing
- semantic document representation
- syntax highlighting policy
- table-of-contents/document-outline generation
- document state
- image-resolution policy
- image cache/scheduling policy
- settings model
- editing model when implemented
- error model
- tests
- reusable application logic

Platform layers may own platform-specific behavior such as:

- native windows
- title bars
- dialogs
- clipboard integration
- filesystem/document-provider access
- file watching implementation
- platform-specific shortcuts
- taskbar/window-manager behavior
- application packaging

Windows-specific code must not unnecessarily leak into the shared document model.

Linux support should reuse suitable shared logic and use the approved Qt 6 Widgets desktop
frontend. Platform-specific behavior must remain behind deliberate boundaries.

Android must be treated as a mobile platform, not as a desktop window squeezed onto a phone.

A future Android UI should be touch-first and compact while preserving Moonmark's recognizable visual identity and renderer quality.

Android presentation technology has not been selected. It must remain touch-first and must comply with the approved language/runtime policy unless the user explicitly approves an exception.

Do not transplant desktop-only concepts such as:

- F11
- desktop custom caption behavior
- tiny desktop toolbar layouts
- Win32 window-state rules

directly into Android.

Android document access may use mechanisms such as document-provider handles or `content://` URIs rather than ordinary filesystem paths.

Therefore, do not make the shared core fundamentally depend on every document always being represented by a permanent normal Windows/Linux path.

This does NOT require implementing Android storage abstractions now. Avoid premature overengineering; simply do not make avoidable assumptions that would block the future target.

Do not select or introduce an Android UI framework, Linux UI framework, cross-platform framework, runtime, or packaging architecture without explicit user approval.

Future platform support is an intended roadmap, not permission for an agent to perform an unsolicited framework migration.

---

# 27B. Integrated optional editor roadmap

Moonmark remains a **renderer/viewer first**.

Read-only viewing remains a complete and valid mode.

A future editor is planned as an **integrated optional capability inside Moonmark**.

It is not planned as a separate application.

Conceptually it behaves more like an add-on/module layered onto Moonmark's viewer and document architecture:

- integrated into the same application
- optional/toggleable
- not required for normal viewing
- should not dominate startup or viewer behavior when unused

At minimum, editing is intended for future Windows and Linux support.

Android editing is not locked in yet and must not be assumed without user approval.

The editing system should reuse Moonmark's shared document and rendering architecture rather than replacing the viewer with an editor-first architecture.

The editor model/source-buffer architecture must remain framework-neutral where practical.

Preview must reuse Moonmark's normal document renderer/presentation path rather than creating a second unrelated Markdown renderer.

Any source-editor component used by Moonmark must comply with the zero-paid / zero-ads dependency rule. Do not require a commercial rich-text/editor control merely to add editing.

Planned editing directions discussed by the user include:

- raw/source Markdown editing
- explicit preview switching
- a preview button in the application UI
- a keyboard shortcut for switching/previewing
- a MarkText-like editing mode where editing and rendered presentation feel closely integrated
- configuration in settings for the preferred editing/preview style

`Ctrl+Shift+V` has been discussed as a possible preview/switch shortcut.

Treat specific keybindings and exact editor UI as planned direction, not an immutable contract, unless the user later locks them.

Do not implement the editor merely because these roadmap rules exist.

When editor work begins, ask before making major choices such as:

- editor widget/engine
- source-buffer architecture
- live-preview strategy
- text-editing framework
- Android editor behavior
- conflict-resolution UX

Viewer-first performance and simplicity must remain intact.

An unused editor feature must not force normal viewing to:

- keep writable file handles open
- load heavy editor components unnecessarily
- reparse documents on every irrelevant UI event
- adopt a workspace/vault model

The editor is an integrated capability built on top of Moonmark, not a reason to redefine Moonmark.

---

# 28. Moonmark identity

The application name is:

**Moonmark**

Do not rename it.

Do not revert to MoonMD.

Preserve the approved Moonmark logo and relevant assets.

---

# 29. Git safety

The primary branch is:

    main

The user may work directly on `main`.

Before making significant repository changes:

    git status
    git branch --show-current
    git log -1 --oneline

Fetch remote state before assuming `main` has not changed when a remote exists.

Preserve user work.

Do NOT:

- force push
- hard reset away user work
- discard unrelated modifications
- assume main is unchanged
- overwrite user commits
- delete unknown untracked files simply because they are untracked

Temporary branches or PRs are allowed only when genuinely useful.

Do not create them unnecessarily.

If a task uses a temporary branch, return to `main` when finished unless explicitly instructed otherwise.

---

# 30. Existing code is not automatically authoritative

Previous Moonmark implementations may contain architecture or design decisions that are no longer approved.

Existing code must not be treated as permission to preserve:

- stale C++/Qt/QTextDocument decisions from the older implementation without evaluating them
- CMake/MSBuild-first workflow
- C#/Avalonia/.NET architecture from the discontinued migration
- CLR/`hostfxr` hosting
- framework-specific types leaking into Moonmark's shared semantic/application core
- paid/Pro/subscription-gated required UI dependencies
- blue UI
- narrow centered layout
- broken maximize behavior
- broken F11 behavior
- browser/web technologies

The obsolete C#/Avalonia/.NET source and runtime integration were removed after the approved replacement reached verified parity. Preserve its behavioral history in documentation and tests, but do not restore the implementation.

Do not restore the obsolete Qt implementation wholesale. Build the approved current Qt Widgets and
QTextDocument architecture around Moonmark's evolved requirements. Do not restore CXX-Qt without
separate approval.

Current user requirements and this AGENTS.md take precedence over stale implementation choices.

---

# 31. Never hide incomplete work

Be precise about what actually works.

Do not report:

- "fullscreen fixed" when only maximize works
- "Cargo-driven" when normal build/run/verification still requires an unrelated manual workflow
- "no blue" without checking actual UI defaults
- "runtime-independent" while an unapproved runtime remains required
- "image performance fixed" without testing image-heavy Markdown

If something remains incomplete, say so clearly.

---

# 32. Required checks after meaningful changes

Run relevant checks whenever possible.

The Cargo-facing workflow is authoritative for normal project verification:

    cargo fmt --check
    cargo check
    cargo test

For release-sensitive changes:

    cargo build --release

The discontinued C#/Avalonia build integration has been removed. Cargo verification must not invoke
the CLR, `hostfxr`, `dotnet`, NuGet, or Avalonia.

For UI/window changes, also perform manual behavior verification where possible.

For document-renderer work, relevant verification should include, where practical:

- mixed inline formatting and selectable text
- headings and paragraph hierarchy
- links and inline code
- code blocks, including long lines and syntax highlighting
- nested lists and task lists
- blockquotes and horizontal rules
- tables
- local images
- table of contents / outline behavior
- large documents containing hundreds or thousands of blocks
- image-heavy documents
- rapid scrolling and repeated resizing
- memory usage and unnecessary UI-object growth

Performance-sensitive document/core tests should remain runnable headlessly where UI is not required.

Do not skip relevant tests silently.

If a test cannot be run, explain why.

---

# 33. Architecture-change stop condition

If you believe any of the following needs to change:

- Rust as Moonmark's primary shipped implementation language
- Python and PowerShell 7 as the other generally approved execution ecosystems
- Cargo as the primary build/check/test/run interface
- Rust as the launcher/entry point
- current Windows native desktop architecture
- Qt 6 Widgets and QTextDocument as the approved Windows/Linux desktop frontend
- the framework-neutral presentation boundary
- the decision to exclude C#/Avalonia/.NET and CXX-Qt
- the runtime policy
- the zero-paid / zero-ads dependency rule
- supported-platform strategy
- selection of a different Linux or Android UI framework
- future editor architecture
- Markdown parser architecture
- rendering engine
- window-state model
- storage architecture
- runtime model
- approved color direction

STOP.

Ask the user first.

Provide:

1. the problem,
2. why the current choice is inadequate,
3. your proposed replacement,
4. alternatives,
5. tradeoffs.

Wait for explicit approval.

---

# 34. Final rule

When unsure whether something is a minor implementation detail or a major project decision:

**ask instead of assuming.**

The project should never undergo a surprise architecture, language, framework, runtime, or visual-direction rewrite.

Moonmark's current locked direction is:

**Rust + C++ as Moonmark's approved first-class implementation languages + Python and PowerShell 7 approved for support/tooling + no other runtime or language without specific explicit approval + Cargo as the top-level build/check/test/run interface + Qt 6 Widgets/QTextDocument as the approved Windows/Linux desktop frontend + no C#/Avalonia/.NET or CXX-Qt shipping architecture + no paid/Pro/subscription/license-key/ad-supported dependency required for Moonmark functionality + reasonably compact installer and portable distributions + Windows-first today, not Windows-only + Linux intended + Android intended + portable shared logic where practical + native desktop presentation + viewer-first with a future integrated optional editor + ordinary external files that Moonmark uses without unnecessarily owning or locking + no browser runtime + wide high-quality Markdown rendering + correct independent maximize/F11 behavior on Windows + achromatic black/lunar application UI + absolutely no blue in Moonmark UI surfaces.**

Important Moonmark semantics, presentation concepts, file behavior, editor concepts, and application logic must remain understandable and testable independently of the current frontend wherever practical.

Syntax-highlighted source code may use a restrained approved semantic color palette while the surrounding Moonmark application UI remains achromatic. The hard prohibition on blue/cyan/teal remains unless the user explicitly changes it.
