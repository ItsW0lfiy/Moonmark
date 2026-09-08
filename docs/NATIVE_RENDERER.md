# Native renderer

Comrak parses CommonMark plus enabled GFM extensions in Rust. Moonmark converts the AST into its own semantic `DocumentModel`, then a compact framework-neutral `PresentationDocument`. The C++ adapter constructs QTextDocument content with native Qt APIs. Markdown is never rendered to HTML, and raw HTML is never executed.

Supported presentation includes paragraphs, H1-H6, soft/hard line breaks, bold, italic, combined emphasis, strikethrough, inline code, links/autolinks, heading anchors, ordered/unordered/nested/task lists, blockquotes, horizontal rules, fenced code, GFM tables, local images, inert raw HTML, basic footnote text, and frontmatter as inert source.

Fenced code uses one QTextFrame with a graphite surface, language metadata, a native separator, padding, preserved whitespace, grouped syntax spans, and a Copy action. Rust performs syntax classification once while building presentation data. Unknown languages use a plain monospace span. The palette uses warm amber, sage, orange, cream, red, and neutral silver—never blue/cyan/teal.

QTextEdit supplies continuous mouse and keyboard selection across document blocks. Ctrl+A selects the full native document. Moonmark intercepts Ctrl+C to preserve document order while removing QTextDocument object/table separator characters. Links activate only on a click gesture; drag and Shift-selection do not activate them. Qt may select a link label on release, so label selection is not used as a substitute for gesture tracking. Fragment links scroll to headings.

Zoom changes native font/layout metrics and responsive document margins without parsing or rebuilding semantics. Resize reflows the existing QTextDocument. Long code currently uses the document viewport's horizontal scrollbar rather than a per-block scrollbar.

Raw HTML appears as inert source text. Scripts, event handlers, iframes, styles, and remote embeds are not interpreted.

Tables use native borderless cells with horizontal separators, retained left/center/right alignment, and unboxed monospace inline code. Quote markers are a contained paint overlay; Qt continues to own the text. Structural paragraphs around tables/code use minimal height to avoid oversized gaps.

Known limitations: QTextFrame has no border-radius property, so the current code surface is square; square corners are accepted for this milestone rather than simulated. Footnote navigation is limited, frontmatter is not a metadata panel, GitHub alerts use ordinary quote presentation, and tables inherit QTextDocument's native selection/copy semantics rather than spreadsheet behavior.
