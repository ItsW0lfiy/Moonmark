use moonmark::markdown::{Block, Inline, parse, to_presentation};
use moonmark::presentation::PresentationMetrics;
use moonmark::settings::Settings;
use moonmark::theme::palette::ALL_DEFAULT_COLOURS;
use moonmark::window::{WindowMode, WindowState};

#[test]
fn required_markdown_constructs_reach_the_semantic_model() {
    let model = parse(
        "# Heading\n\n- [x] task\n  - nested\n\n[link](https://example.invalid) ![image](images/a.png)\n\n|A|B|\n|-|-|\n|1|2|\n\n```rust\ncode\n```",
    );
    assert!(
        model
            .blocks
            .iter()
            .any(|block| matches!(block, Block::Heading { .. }))
    );
    assert!(
        model
            .blocks
            .iter()
            .any(|block| matches!(block, Block::List { .. }))
    );
    assert!(
        model
            .blocks
            .iter()
            .any(|block| matches!(block, Block::Table { .. }))
    );
    assert!(
        model
            .blocks
            .iter()
            .any(|block| matches!(block, Block::CodeBlock { .. }))
    );
    let paragraph = model
        .blocks
        .iter()
        .find_map(|block| match block {
            Block::Paragraph(inlines) => Some(inlines),
            _ => None,
        })
        .unwrap();
    assert!(
        paragraph
            .iter()
            .any(|inline| matches!(inline, Inline::Link { .. }))
    );
    assert!(
        paragraph
            .iter()
            .any(|inline| matches!(inline, Inline::Image { .. }))
    );
}

#[test]
fn maximize_and_fullscreen_are_independent_states() {
    let mut state = WindowState::default();
    state.toggle_maximize_model();
    state.enter_fullscreen_model();
    assert_eq!(state.mode, WindowMode::BorderlessFullscreen);
    assert!(state.leave_fullscreen_model());
    assert_eq!(state.mode, WindowMode::Maximized);
}

#[test]
fn default_palette_is_entirely_achromatic() {
    assert!(
        ALL_DEFAULT_COLOURS
            .iter()
            .all(|colour| colour.is_achromatic())
    );
}

#[test]
fn presentation_heading_targets_match_unique_toc_anchors() {
    let model = parse("# Repeat\n\n## Repeat\n\n[Jump](#repeat-1)");
    let (presentation, images) = to_presentation(
        &model,
        std::path::Path::new("C:/Moonmark/document.md"),
        PresentationMetrics {
            revision: 1,
            ..PresentationMetrics::default()
        },
        &Settings::default(),
    );
    assert!(images.is_empty());
    assert_eq!(
        presentation
            .toc
            .iter()
            .map(|entry| entry.anchor.as_str())
            .collect::<Vec<_>>(),
        ["repeat", "repeat-1"]
    );
    assert_eq!(
        presentation
            .commands
            .iter()
            .filter(|command| command.kind == 2)
            .map(|command| command.target.as_str())
            .collect::<Vec<_>>(),
        ["repeat", "repeat-1"]
    );
}
