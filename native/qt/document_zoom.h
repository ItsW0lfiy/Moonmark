#pragma once

#include <QPointer>
#include <QTextDocument>
#include <QTextFrame>
#include <QTextTable>
#include <vector>

namespace moonmark::qt {

// Immutable 100% presentation metrics; positions stay valid in the read-only document.
class DocumentZoom {
public:
    void capture(QTextDocument* document);
    void apply(int percent);
    [[nodiscard]] bool matches(int percent) const;

private:
    struct Span { int position; int length; QTextCharFormat format; };
    struct Block { int position; QTextBlockFormat format; QTextCharFormat character; };
    struct Frame { QPointer<QTextFrame> frame; QTextFrameFormat format; };
    struct Cell { QTextTableCell cell; QTextTableCellFormat format; };
    void captureFrame(QTextFrame* frame);
    QPointer<QTextDocument> document_;
    QFont font_;
    std::vector<Span> spans_;
    std::vector<Block> blocks_;
    std::vector<Frame> frames_;
    std::vector<Cell> cells_;
};

} // namespace moonmark::qt
