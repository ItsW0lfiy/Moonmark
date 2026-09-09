# Native renderer

Comrak parses CommonMark and enabled GFM extensions in Rust. Moonmark converts its semantic DocumentModel to framework-neutral PresentationDocument commands. The Qt adapter builds QTextDocument through native cursor, block, frame, table, and character APIs. Markdown is never converted to HTML; raw HTML remains inert source.

Supported presentation includes H1–H6, paragraphs, soft/hard breaks, bold/italic/combined emphasis/strike, inline code, links/autolinks, anchors/outline, nested ordered/unordered/task lists, quotes, rules, fences, syntax highlighting, tables with alignment, and local images. Footnotes are basic text, frontmatter remains inert source, and alerts use quote presentation.

Code remains one graphite QTextFrame with metadata, a real separator, preserved whitespace, and Copy. Rust classifies syntax once during presentation construction; unknown languages fall back to plain monospace. The palette uses warm amber, sage, orange, cream, red, and silver, never blue/cyan/teal.

QTextEdit supplies continuous mouse/keyboard selection. Ctrl+A selects the native document; Ctrl+C preserves document order while stripping object/table separator characters. Links activate on click gestures, not drag or Shift-selection. Heading links and the sidebar navigate the same native anchors. Qt's accessible text interface remains present.

Tables retain native cell selection and left/center/right alignment. Their graphite header, faint outer/horizontal lines, weaker vertical lines, and modest padding support scanning. Numeric columns in mixed tables have content-sized widths; remaining columns use Qt's text layout and available desktop space. Inline-code character backgrounds stay within the text flow.

## Percentage zoom and lifecycle

DocumentZoom captures immutable 100% native formats once after construction. Zoom updates existing text/block/frame/cell formats and image display geometry, with no Markdown parse, semantic rebuild, presentation regeneration, or QTextDocument replacement. It preserves the cursor selection and a top-visible text anchor. Width changes simply reflow the same document; decoded images are reused.

The old default-font zoom path was ineffective because the renderer assigns explicit text sizes. It also approximated percentages as whole font-point steps. Dev.4 replaces it with proportional scaling of the actual native formats, including code/table/list metrics. Ctrl+wheel and keyboard shortcuts use the same path as the percentage menu.

Qt has no CSS-style radius/padding API for inline character backgrounds and no native rounded QTextFrame border. Current frames remain square and inline backgrounds tight; layout, native selection, accessibility, and copy are not compromised to simulate decoration. Long code uses the document's horizontal scrollbar rather than per-block overflow. Very large documents still reflow synchronously during zoom; measurements are documented in dev.4 validation.
