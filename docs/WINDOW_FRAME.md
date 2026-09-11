# Window behavior

Dev.6 does not change the established Normal, Maximized, and BorderlessFullscreen state model. Multi-document activation changes only the stacked document view and title metadata. Window resize, maximize, restore, and F11 do not reopen files or recreate retained sessions.

Discrete mouse-wheel input uses one reusable floating-point, elapsed-time controller. Repeated input extends the live target without resetting velocity; direction changes alter that same trajectory. A critically damped analytic step uses real elapsed time and imposes no fixed pixels-per-frame cap, so rapid input can move far while retaining coherent velocity. Qt Widgets' raster backing store has no dependable presented-frame callback. A precise timer requests viewport updates at 16 ms normally and 8 ms only while distance/velocity is high; state advances in the resulting paint event. The faster request cadence prevents a timer/display phase miss from dropping a physical 60 Hz presentation while avoiding high-rate idle or slow-motion work.

Precision pixel/touchpad input, scrollbar dragging, keyboard paging/Home/End, outline activation, heading links, tab restoration, and layout-anchor correction are direct. Mouse or keyboard takeover cancels wheel motion. Sidebar width remains a native property animation but reverses from its actual partial width. `MOONMARK_REDUCED_MOTION=1` makes Moonmark-owned wheel and outline motion direct.

`--smoke-scroll-profile` enables a bounded in-memory frame trace and reports input-to-first-paint, paint-interval p50/p95/p99/worst, threshold counts over 16.67/25/33.3/50 ms, paint cost, scrollbar/value-change cost, and image scan/delivery cost. It emits one summary rather than logging each frame. `MOONMARK_SCROLL_PROFILE_OUTPUT` optionally writes that summary to a file, which is useful for the Windows GUI-subsystem Release executable.

Moonmark uses one frameless Qt Widgets window with a custom achromatic title bar. The dedicated `MoonTitleBar` component owns title-region move, double-click, and system-menu input; Windows-specific hit testing remains contained in the Qt adapter. Document/core state remains independent.

Rust tracks three semantic modes: `Normal`, `Maximized`, and `BorderlessFullscreen`.

Normal maximize uses the operating system's standard maximize/restore operation. Moonmark chrome remains visible and the Windows work area keeps the taskbar visible. F11 is separate same-window borderless fullscreen: Moonmark preserves the prior Normal/Maximized mode, hides its title bar (including document actions) and optional diagnostics row, fills the current screen through Qt, and restores the saved mode on F11 or Escape.

The Windows adapter handles edge/corner `WM_NCHITTEST`, system move, caption actions, Alt+Space, and the right-click system menu. Double-click title-bar maximize/restore is implemented by the title-bar widget. The maximize button intentionally does not opt into the previously rejected custom Snap Layout hover treatment.

Resize, zoom, maximize, F11, and restoration operate on the existing QTextDocument. The native smoke matrix asserts zero parse, file-load, document-construction, and image-request deltas across those transitions.

Physical Windows checks still required before release: Windows 10/11 snapping, mixed-DPI multi-monitor movement, exact taskbar work-area behavior, title-bar drag/resize cursors, system-menu details, focus, and Normal-to-F11 geometry restoration on more than one monitor.

Dev.4 adds a collapsible sidebar and 48px breadcrumb header. F11 hides the sidebar as well as the header; leaving it restores the user's sidebar preference subject to responsive width. The navigation smoke covers keyboard outline activation, automatic narrow-window collapse, and fullscreen chrome visibility without document reconstruction.
