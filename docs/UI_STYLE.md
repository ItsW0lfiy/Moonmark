# UI style — New Moon, 0.1.0-dev.3

Moonmark is document-first: space, alignment, and typography carry hierarchy before borders. Chrome is achromatic black/graphite/silver. Syntax colors and the restrained red close hover are the only deliberate exceptions; neither introduces blue/cyan/teal.

## Window and commands

One 40-logical-pixel titlebar replaces dev.2's 46px titlebar plus 42px command strip. The embedded icon is shown at 18px, followed by a left-aligned eliding filename. The remaining title area is draggable. Open and an unboxed zoom group appear when a document is open. The ellipsis menu keeps Reload, Reset zoom, Fullscreen, and Diagnostics discoverable; Ctrl+O, F5, F11/Escape, and F12 remain available.

Caption controls have 44×40px hit targets, native painted symbols, quiet resting surfaces, hover/pressed feedback, and visible keyboard focus. The close hover is muted red. The compact empty state uses a 32px embedded symbol, small wordmark, drag/drop hint, Open action, and Ctrl+O hint. No large landing-page logo or permanent footer remains.

## Document rhythm

The canvas uses available desktop width with responsive side margins from 20 to 48px, not a fixed article-width limit. The default body is 12.75pt with 150% native line height. Heading scales remain distinct and application-sized. Lists use hanging markers and explicit tab stops; task glyphs remain integrated with native text.

Tables have no outer or vertical grid and no filled cell cards. Header weight and subtle horizontal rules provide structure. Native cell padding is 6px vertically and up to 12px horizontally, with 125% line height. Comrak column alignment survives the framework-neutral model. Inline code in tables is plain monospace without a background.

Code stays in one borderless graphite QTextFrame with 14px padding, a small neutral language label, lightweight Copy link, real separator, syntax spans, and preserved source whitespace. Structural paragraphs around native frames have minimal height; they no longer create extra body-line gaps. Square corners are intentional rather than simulated rounding. Long lines retain the viewport's horizontal scrollbar.

Quotes use muted text and a 2px neutral marker painted alongside Qt's native blocks. The quote text itself is never custom-painted. No rectangular quote fill remains. Rules are 1px neutral dividers. Inline code outside tables has a subdued graphite background. Image errors/loading states use a restrained left marker and text rather than a bordered card.

## States and accessibility

Qt Fusion plus a neutral QPalette overrides accent, links, selection, and control states. Sparse widget styling supplies hover/focus treatments. Scrollbars retain 12px tracks, subdued resting handles, and clearer hover contrast. F12 diagnostics remain secondary. Native file dialogs intentionally retain the operating system's UI.

QTextDocument owns layout, selection, and accessibility. Tests exercise whole-document Ctrl+A/Ctrl+C, code Copy, table formats/alignment, neutral palette roles, and the Qt accessible-text interface. This is not a substitute for physical UI Automation/screen-reader or mixed-DPI validation.

See [dev.3 validation](DEV3_VALIDATION.md) for screenshot findings, commands, results, and remaining checks.
