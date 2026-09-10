# Moonmark roadmap

The roadmap records intended direction, not release dates or promises.

## 0.1.0-dev.6 — current

- retained multi-document viewing in one process
- literal `.txt` viewing alongside Markdown
- smooth, interruptible document navigation and restrained native motion
- code-block and table visual refinement
- GPL-3.0-only project licensing and release documentation

## 0.1.0-dev.7 — Windows installer

Select an installer engine separately. Install the existing Moonmark package under Program Files with Installed Apps/uninstall registration, Start Menu and optional desktop shortcuts, safe upgrades, and Windows Open With/file associations for supported document types. Include Moonmark, required Qt/native libraries, plugins, CRT files, assets, notices, and licenses. Keep the portable ZIP as a distinct distribution using the same application architecture.

## 0.1.0-dev.8 — GitHub Releases updater

Check asynchronously at most once near startup, remain quiet when current, and present a restrained opt-in prompt when an update exists. Download the complete installer, verify SHA-256, and keep stable, preview, and development channels explicit. Treat portable updates separately. Authenticode signing may be added later if practical and explicitly configured.

## Around 0.1.0-dev.9

Prepare optional WinGet publication with the conceptual identifier `ItsW0lfiy.Moonmark`. Validate installer metadata, silent install/uninstall behavior, upgrade continuity, hashes, and publisher identity before submission.

## Later work

- decide how shell/file-association opens behave when Moonmark is already running: independent process, activate an existing window, or open a new document in a chosen window
- Linux desktop build, packaging, accessibility, and integration validation
- Android-specific frontend and document-provider handling
- optional integrated editor that reuses Moonmark's parser and renderer
- settings/theme infrastructure, including reduced-motion preference
- single-executable packaging research without compromising Qt licensing or maintainability

Moonmark remains viewer-first and file-oriented. Installer, updater, WinGet, Android, editor, and single-instance IPC are not implemented in dev.6.
