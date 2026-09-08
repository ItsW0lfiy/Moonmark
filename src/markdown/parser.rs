use comrak::nodes::{AstNode, NodeValue};
use comrak::{Arena, Options, parse_document};

use super::model::{Block, DocumentModel, Inline, ListItem, TableRow, plain_text};

pub fn parse(source: &str) -> DocumentModel {
    let arena = Arena::new();
    let mut options = Options::default();
    options.extension.strikethrough = true;
    options.extension.table = true;
    options.extension.autolink = true;
    options.extension.tasklist = true;
    options.extension.footnotes = true;
    options.extension.front_matter_delimiter = Some("---".into());
    let root = parse_document(&arena, source, &options);
    DocumentModel {
        blocks: block_children(root),
    }
}

fn block_children<'a>(node: &'a AstNode<'a>) -> Vec<Block> {
    node.children().filter_map(parse_block).collect()
}

fn parse_block<'a>(node: &'a AstNode<'a>) -> Option<Block> {
    let value = node.data.borrow().value.clone();
    match value {
        NodeValue::Paragraph => Some(Block::Paragraph(inline_children(node))),
        NodeValue::Heading(heading) => Some(Block::Heading {
            level: heading.level,
            content: inline_children(node),
        }),
        NodeValue::BlockQuote => Some(Block::Quote(block_children(node))),
        NodeValue::List(list) => {
            let items = node
                .children()
                .filter_map(|item| match &item.data.borrow().value {
                    NodeValue::Item(_) => {
                        let blocks = block_children(item);
                        let checked = first_task_state(&blocks);
                        Some(ListItem { checked, blocks })
                    }
                    NodeValue::TaskItem(task) => Some(ListItem {
                        checked: Some(task.symbol.is_some()),
                        blocks: block_children(item),
                    }),
                    _ => None,
                })
                .collect();
            Some(Block::List {
                ordered: list.list_type == comrak::nodes::ListType::Ordered,
                start: list.start as u64,
                items,
            })
        }
        NodeValue::CodeBlock(code) => Some(Block::CodeBlock {
            language: code
                .info
                .split_whitespace()
                .next()
                .unwrap_or_default()
                .to_owned(),
            source: code.literal,
        }),
        NodeValue::ThematicBreak => Some(Block::HorizontalRule),
        NodeValue::Table(_) => Some(parse_table(node)),
        NodeValue::HtmlBlock(html) => Some(Block::RawHtml(html.literal)),
        NodeValue::FrontMatter(source) => Some(Block::RawHtml(source)),
        NodeValue::FootnoteDefinition(definition) => {
            let mut content = vec![Inline::Strong(vec![Inline::Text(format!(
                "{}: ",
                definition.name
            ))])];
            for block in block_children(node) {
                if let Block::Paragraph(inlines) = block {
                    content.extend(inlines);
                }
            }
            Some(Block::Paragraph(content))
        }
        _ => None,
    }
}

fn parse_table<'a>(node: &'a AstNode<'a>) -> Block {
    let mut rows = node.children().map(parse_table_row);
    Block::Table {
        header: rows.next().unwrap_or_default(),
        rows: rows.collect(),
    }
}

fn parse_table_row<'a>(node: &'a AstNode<'a>) -> TableRow {
    TableRow {
        cells: node.children().map(inline_children).collect(),
    }
}

fn inline_children<'a>(node: &'a AstNode<'a>) -> Vec<Inline> {
    node.children().filter_map(parse_inline).collect()
}

fn parse_inline<'a>(node: &'a AstNode<'a>) -> Option<Inline> {
    let value = node.data.borrow().value.clone();
    match value {
        NodeValue::Text(text) => Some(Inline::Text(text.into_owned())),
        NodeValue::Emph => Some(Inline::Emphasis(inline_children(node))),
        NodeValue::Strong => Some(Inline::Strong(inline_children(node))),
        NodeValue::Strikethrough => Some(Inline::Strikethrough(inline_children(node))),
        NodeValue::Code(code) => Some(Inline::Code(code.literal)),
        NodeValue::Link(link) => Some(Inline::Link {
            destination: link.url,
            content: inline_children(node),
        }),
        NodeValue::Image(link) => {
            let children = inline_children(node);
            Some(Inline::Image {
                source: link.url,
                title: link.title,
                alt: plain_text(&children),
            })
        }
        NodeValue::SoftBreak => Some(Inline::SoftBreak),
        NodeValue::LineBreak => Some(Inline::HardBreak),
        NodeValue::HtmlInline(source) => Some(Inline::RawHtml(source)),
        NodeValue::TaskItem(marker) => Some(Inline::TaskMark(marker.symbol.is_some())),
        NodeValue::FootnoteReference(reference) => {
            Some(Inline::Text(format!("[{}]", reference.name)))
        }
        _ => {
            let children = inline_children(node);
            (!children.is_empty()).then_some(Inline::Text(plain_text(&children)))
        }
    }
}

fn first_task_state(blocks: &[Block]) -> Option<bool> {
    let Block::Paragraph(inlines) = blocks.first()? else {
        return None;
    };
    match inlines.first() {
        Some(Inline::TaskMark(value)) => Some(*value),
        _ => None,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parses_gfm_structure_without_html_rendering() {
        let document =
            parse("# Heading\n\n- [x] task\n  - nested\n\n| A | B |\n|---|---|\n| 1 | 2 |\n");
        assert!(matches!(
            document.blocks[0],
            Block::Heading { level: 1, .. }
        ));
        assert!(
            document
                .blocks
                .iter()
                .any(|block| matches!(block, Block::List { .. }))
        );
        let Block::List { items, .. } = &document.blocks[1] else {
            panic!("expected task list");
        };
        assert_eq!(items[0].checked, Some(true));
        assert!(!items[0].blocks.is_empty());
        assert!(
            document
                .blocks
                .iter()
                .any(|block| matches!(block, Block::Table { .. }))
        );
    }

    #[test]
    fn preserves_links_images_and_code() {
        let document = parse(
            "[Moonmark](https://example.invalid) ![alt](images/a.png) `code`\n\n```rust\nlet x = 1;\n```",
        );
        assert_eq!(document.blocks.len(), 2);
        assert!(matches!(document.blocks[1], Block::CodeBlock { .. }));
    }
}
