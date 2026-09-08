# Next steps

The native Rust/C++/Qt renderer foundation and `0.1.0-dev.3` document-first Qt redesign are in place. The next milestone should harden physical Windows integration without changing this architecture:

1. Physically validate title-bar dragging, edge/corner resize, taskbar behavior, Alt+Space, snapping, mixed-DPI multi-monitor movement, and both fullscreen restoration paths on Windows 10 and Windows 11.
2. Review the dev.3 visual baseline with the user, then improve user-facing external-change/reload feedback. The native watcher smoke already covers refresh.
3. Improve QTextDocument presentation limitations: per-code-block horizontal overflow, footnote navigation, frontmatter presentation, and GitHub alert styling.
4. Add native TOC/outline UI only after the viewer foundation remains stable.
5. Produce a real installer once installer technology and file-association/update policy are explicitly approved.
6. Validate a Linux build and package with a supported Qt 6 distribution, including AT-SPI accessibility and desktop integration.
7. Decide Moonmark's own distribution license and complete legal review of Qt LGPL obligations before publishing binaries.

Android and the optional integrated editor remain later, separately designed milestones.
