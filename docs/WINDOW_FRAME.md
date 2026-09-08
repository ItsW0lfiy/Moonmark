# Window behavior

Moonmark uses one frameless Qt Widgets window with a custom achromatic title bar. The dedicated `MoonTitleBar` component owns title-region move, double-click, and system-menu input; Windows-specific hit testing remains contained in the Qt adapter. Document/core state remains independent.

Rust tracks three semantic modes: `Normal`, `Maximized`, and `BorderlessFullscreen`.

Normal maximize uses the operating system's standard maximize/restore operation. Moonmark chrome remains visible and the Windows work area keeps the taskbar visible. F11 is separate same-window borderless fullscreen: Moonmark preserves the prior Normal/Maximized mode, hides its title bar, contextual command strip, and optional diagnostics row, fills the current screen through Qt, and restores the saved mode on F11 or Escape.

The Windows adapter handles edge/corner `WM_NCHITTEST`, system move, caption actions, Alt+Space, and the right-click system menu. Double-click title-bar maximize/restore is implemented by the title-bar widget. The maximize button intentionally does not opt into the previously rejected custom Snap Layout hover treatment.

Resize, zoom, maximize, F11, and restoration operate on the existing QTextDocument. The native smoke matrix asserts zero parse, file-load, document-construction, and image-request deltas across those transitions.

Physical Windows checks still required before release: Windows 10/11 snapping, mixed-DPI multi-monitor movement, exact taskbar work-area behavior, title-bar drag/resize cursors, system-menu details, focus, and Normal-to-F11 geometry restoration on more than one monitor.
