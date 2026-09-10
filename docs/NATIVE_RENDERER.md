# Native renderer

Comrak parses CommonMark and enabled GFM extensions in Rust. Moonmark converts its semantic DocumentModel to framework-neutral PresentationDocument commands. The Qt adapter builds QTextDocument through native cursor, block, frame, table, and character APIs. Markdown is never converted to HTML; raw HTML remains inert source.

Supported presentation includes H1–H6, paragraphs, soft/hard breaks, bold/italic/combined emphasis/strike, inline code, links/autolinks, anchors/outline, nested ordered/unordered/task lists, quotes, rules, fences, syntax highlighting, tables with alignment, and local images. Footnotes are basic text, frontmatter remains inert source, and alerts use quote presentation.

Code remains one graphite QTextFrame with metadata, a real separator, preserved whitespace, and Copy. Rust classifies syntax once during presentation construction; unknown languages fall back to plain monospace. The palette uses warm amber, sage, orange, cream, red, and silver, never blue/cyan/teal.

QTextEdit supplies continuous mouse/keyboard selection. Ctrl+A selects the native document; Ctrl+C preserves document order while stripping object/table separator characters. Links activate on click gestures, not drag or Shift-selection. Heading links and the sidebar navigate the same native anchors. Qt's accessible text interface remains present.

Tables retain native cell selection and left/center/right alignment. Their graphite header, faint outer/horizontal lines, weaker vertical lines, and modest padding support scanning. Numeric columns in mixed tables have content-sized widths; remaining columns use Qt's text layout and available desktop space. Inline-code character backgrounds stay within the text flow.

## Percentage zoom and lifecycle

DocumentZoom captures immutable 100% native formats once after construction. Zoom updates existing text/block/frame/cell formats and image display geometry, with no Markdown parse, semantic rebuild, presentation regeneration, or QTextDocument replacement. It preserves the cursor selection and a top-visible text anchor. Width changes simply reflow the same document; decoded images are reused.

The old default-font zoom path was ineffective because the renderer assigns explicit text sizes. It also approximated percentages as whole font-point steps. Dev.4 replaces it with proportional scaling of the actual native formats, including code/table/list metrics. Ctrl+wheel and keyboard shortcuts use the same path as the percentage menu.

Qt has no CSS-style radius/padding API for inline character backgrounds. Dev.5 extends their native graphite surface with a small rounded painted edge using Qt's own line/cursor geometry. The underlying characters, shaping, wrapping, and accessibility remain native; no padding spaces or image/text objects are inserted. Selected spans defer entirely to native selection painting. This is decorative breathing room, not a true inline box model: it does not reserve additional advance width between tightly adjacent characters. Trailing wrapping whitespace is excluded from decoration.

Standalone images use 100% line height and explicit block margins. The previous 125% line height added unintended leading proportional to image height (115.8px for a 463px image), despite the image block's bounding rectangle reporting the correct image height. The regression compares the following block's position against the image block's bottom and declared margins, across loading and zoom.

Long code uses the document's horizontal scrollbar rather than per-block overflow. Code frames and the accepted dev.4 shell are unchanged by dev.5. Very large documents still require native format updates and layout during zoom; profiling separates action/paint time from exhaustive verification.
