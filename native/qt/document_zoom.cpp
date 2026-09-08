#include "document_zoom.h"

#include <QTextBlock>
#include <QTextCursor>
#include <cmath>

namespace moonmark::qt {
namespace {
template <typename Format>
Format scaled(Format format, double ratio) {
    for (const auto property : {QTextFormat::FontPointSize, QTextFormat::FontPixelSize,
            QTextFormat::BlockTopMargin, QTextFormat::BlockBottomMargin,
            QTextFormat::BlockLeftMargin, QTextFormat::BlockRightMargin, QTextFormat::TextIndent,
            QTextFormat::FrameTopMargin, QTextFormat::FrameBottomMargin,
            QTextFormat::FrameLeftMargin, QTextFormat::FrameRightMargin, QTextFormat::FramePadding,
            QTextFormat::TableCellTopPadding, QTextFormat::TableCellBottomPadding,
            QTextFormat::TableCellLeftPadding, QTextFormat::TableCellRightPadding}) {
        if (format.hasProperty(property))
            format.setProperty(property, format.doubleProperty(property) * ratio);
    }
    return format;
}
} // namespace

void DocumentZoom::capture(QTextDocument* document) {
    document_ = document;
    font_ = document->defaultFont();
    spans_.clear();
    blocks_.clear();
    frames_.clear();
    cells_.clear();
    for (auto block = document->begin(); block.isValid(); block = block.next()) {
        blocks_.push_back({block.position(), block.blockFormat(), block.charFormat()});
        for (auto it = block.begin(); !it.atEnd(); ++it) {
            const auto fragment = it.fragment();
            if (!fragment.charFormat().isImageFormat())
                spans_.push_back({fragment.position(), fragment.length(), fragment.charFormat()});
        }
    }
    for (auto* frame : document->rootFrame()->childFrames()) captureFrame(frame);
}

void DocumentZoom::captureFrame(QTextFrame* frame) {
    frames_.push_back({frame, frame->frameFormat()});
    if (auto* table = qobject_cast<QTextTable*>(frame)) {
        for (int row = 0; row < table->rows(); ++row)
            for (int column = 0; column < table->columns(); ++column) {
                const auto cell = table->cellAt(row, column);
                cells_.push_back({cell, cell.format().toTableCellFormat()});
            }
    }
    for (auto* child : frame->childFrames()) captureFrame(child);
}

void DocumentZoom::apply(int percent) {
    if (!document_) return;
    const double ratio = percent / 100.0;
    auto font = font_;
    font.setPointSizeF(font_.pointSizeF() * ratio);
    document_->setDefaultFont(font);
    QTextCursor cursor(document_);
    cursor.beginEditBlock();
    for (const auto& block : blocks_) {
        cursor.setPosition(block.position);
        auto format = scaled(block.format, ratio);
        auto tabs = format.tabPositions();
        for (auto& tab : tabs) tab.position *= ratio;
        format.setTabPositions(tabs);
        // Percentage line heights and 1px presentation rules deliberately stay unchanged.
        cursor.setBlockFormat(format);
        cursor.setBlockCharFormat(scaled(block.character, ratio));
    }
    for (const auto& span : spans_) {
        cursor.setPosition(span.position);
        cursor.setPosition(span.position + span.length, QTextCursor::KeepAnchor);
        cursor.setCharFormat(scaled(span.format, ratio));
    }
    for (const auto& frame : frames_)
        if (frame.frame) frame.frame->setFrameFormat(scaled(frame.format, ratio));
    for (auto& cell : cells_) cell.cell.setFormat(scaled(cell.format, ratio));
    cursor.endEditBlock();
}

bool DocumentZoom::matches(int percent) const {
    if (!document_ || spans_.empty()) return false;
    const double ratio = percent / 100.0;
    for (const auto& span : spans_) {
        QTextCursor cursor(document_);
        cursor.setPosition(span.position);
        cursor.setPosition(span.position + span.length, QTextCursor::KeepAnchor);
        const auto actual = cursor.charFormat();
        if (std::abs(actual.fontPointSize() - span.format.fontPointSize() * ratio) > 0.001 ||
            actual.fontWeight() != span.format.fontWeight() ||
            actual.fontItalic() != span.format.fontItalic() ||
            actual.anchorHref() != span.format.anchorHref()) return false;
    }
    return true;
}
} // namespace moonmark::qt
