#include "moonmark_qt.h"
#include "moon_style.h"
#include "moon_title_bar.h"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QAccessible>
#include <QBoxLayout>
#include <QClipboard>
#include <QCloseEvent>
#include <QColor>
#include <QDesktopServices>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QFontDatabase>
#include <QFrame>
#include <QGuiApplication>
#include <QImage>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QPointer>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QScreen>
#include <QScrollBar>
#include <QSettings>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QStyle>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextFrame>
#include <QTextFrameFormat>
#include <QTextImageFormat>
#include <QTextListFormat>
#include <QTextTable>
#include <QTextTableCell>
#include <QTextTableFormat>
#include <QTimer>
#include <QUrl>
#include <QWindow>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include <dwmapi.h>
#include <windows.h>
#include <windowsx.h>
#endif

namespace {

namespace colour = moonmark::style::colour;
constexpr int quote_depth_property = QTextFormat::UserProperty + 1;

namespace command_kind {
constexpr int begin_paragraph = 1;
constexpr int begin_heading = 2;
constexpr int end_block = 3;
constexpr int text = 4;
constexpr int soft_break = 5;
constexpr int hard_break = 6;
constexpr int begin_list = 7;
constexpr int end_list = 8;
constexpr int begin_item = 9;
constexpr int end_item = 10;
constexpr int code_block = 11;
constexpr int horizontal_rule = 12;
constexpr int begin_quote = 13;
constexpr int end_quote = 14;
constexpr int begin_table = 15;
constexpr int begin_row = 16;
constexpr int begin_cell = 17;
constexpr int end_cell = 18;
constexpr int end_row = 19;
constexpr int end_table = 20;
constexpr int image = 21;
constexpr int raw_html = 22;
} // namespace command_kind

namespace text_style {
constexpr int emphasis = 1;
constexpr int strong = 2;
constexpr int strike = 4;
constexpr int code = 8;
constexpr int link = 16;
constexpr int ordered = 32;
constexpr int checked = 64;
constexpr int unchecked = 128;
constexpr int header = 256;
} // namespace text_style

struct Command {
    int kind = 0;
    int level = 0;
    int flags = 0;
    QString text;
    QString target;
    QString extra;
    qint64 number = 0;
    QJsonArray spans;
};

struct ImageOccurrence {
    std::uint32_t id = 0;
    int position = 0;
    bool requested = false;
    bool loaded = false;
    bool failed = false;
    int natural_width = 0;
    int natural_height = 0;
};

QString fromBuffer(const MoonmarkBuffer& buffer) {
    if (buffer.data == nullptr || buffer.len == 0) {
        return {};
    }
    return QString::fromUtf8(reinterpret_cast<const char*>(buffer.data),
                             static_cast<qsizetype>(buffer.len));
}

QJsonObject jsonFromBuffer(const MoonmarkBuffer& buffer) {
    if (buffer.data == nullptr || buffer.len == 0) {
        return {};
    }
    const auto bytes = QByteArray::fromRawData(reinterpret_cast<const char*>(buffer.data),
                                               static_cast<qsizetype>(buffer.len));
    return QJsonDocument::fromJson(bytes).object();
}

QString applicationAssetPath(const QString& relative) {
    const auto packaged = QCoreApplication::applicationDirPath() + QLatin1Char('/') + relative;
    if (QFileInfo::exists(packaged)) {
        return packaged;
    }
    return relative;
}

QIcon applicationIcon() {
#ifdef _WIN32
    QIcon icon;
    const auto module = GetModuleHandleW(nullptr);
    for (const int size : {16, 24, 32, 48, 64, 128, 256}) {
        const auto handle = static_cast<HICON>(LoadImageW(
            module, MAKEINTRESOURCEW(1), IMAGE_ICON, size, size, LR_DEFAULTCOLOR));
        if (handle == nullptr) {
            continue;
        }
        icon.addPixmap(QPixmap::fromImage(QImage::fromHICON(handle)));
        DestroyIcon(handle);
    }
    if (!icon.isNull()) {
        return icon;
    }
#endif
    return QIcon(applicationAssetPath(QStringLiteral("assets/icons/moonmark.ico")));
}

Command parseCommand(const QJsonValue& value) {
    const auto object = value.toObject();
    return Command{object.value("kind").toInt(),
                   object.value("level").toInt(),
                   object.value("flags").toInt(),
                   object.value("text").toString(),
                   object.value("target").toString(),
                   object.value("extra").toString(),
                   object.value("number").toInteger(),
                   object.value("spans").toArray()};
}

QTextCharFormat baseCharacterFormat(double points = 12.75) {
    QTextCharFormat format;
    format.setFontFamilies({QStringLiteral("Segoe UI Variable Text"), QStringLiteral("Segoe UI")});
    format.setFontPointSize(points);
    format.setForeground(QColor(colour::text));
    return format;
}

QTextBlockFormat bodyBlockFormat(int line_height = 150) {
    QTextBlockFormat format;
    format.setTopMargin(1.0);
    format.setBottomMargin(8.0);
    format.setLineHeight(line_height, QTextBlockFormat::ProportionalHeight);
    return format;
}

QImage placeholderImage(const QString& message, int width = 900, int height = 72,
                        bool failed = false) {
    QImage image(width, height, QImage::Format_RGBA8888);
    image.fill(QColor(colour::surface));
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor(colour::border_strong), 2));
    painter.drawLine(1, 10, 1, height - 10);
    painter.setPen(QColor(failed ? colour::secondary : colour::muted));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 11));
    painter.drawText(image.rect().adjusted(22, 12, -22, -12), Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap,
                     message);
    return image;
}

QMimeData* snapshotClipboard() {
    auto* saved = new QMimeData;
    if (const auto* current = QGuiApplication::clipboard()->mimeData()) {
        for (const auto& format : current->formats()) saved->setData(format, current->data(format));
    }
    return saved;
}

class MoonButton final : public QPushButton {
public:
    explicit MoonButton(const QString& text, QWidget* parent = nullptr) : QPushButton(text, parent) {
        setCursor(Qt::PointingHandCursor);
        setMinimumHeight(moonmark::style::metric::control_height);
        setFocusPolicy(Qt::StrongFocus);
    }
};

class ElidingLabel final : public QLabel {
public:
    explicit ElidingLabel(const QString& text, QWidget* parent = nullptr) : QLabel(parent) {
        setFullText(text);
    }

    void setFullText(const QString& text) {
        full_text_ = text;
        setToolTip(text);
        updateElision();
    }

protected:
    void resizeEvent(QResizeEvent* event) override {
        QLabel::resizeEvent(event);
        updateElision();
    }

private:
    void updateElision() {
        QLabel::setText(fontMetrics().elidedText(full_text_, Qt::ElideMiddle, width()));
    }

    QString full_text_;
};

class DocumentView final : public QTextEdit {
public:
    explicit DocumentView(const MoonmarkApiTable* api, void* backend, QWidget* parent = nullptr)
        : QTextEdit(parent), api_(api), backend_(backend) {
        setReadOnly(true);
        setAcceptRichText(false);
        setFrameShape(QFrame::NoFrame);
        setUndoRedoEnabled(false);
        setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard |
                                Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setFocusPolicy(Qt::StrongFocus);
        setAccessibleName(QStringLiteral("Moonmark Markdown document"));
        viewport()->setMouseTracking(true);
        document()->setDefaultStyleSheet({});
        document()->setDocumentMargin(0.0);

        image_poll_.setInterval(15);
        QObject::connect(&image_poll_, &QTimer::timeout, this, [this] { pollImages(); });
        QObject::connect(verticalScrollBar(), &QScrollBar::valueChanged, this,
                         [this] { queueVisibleImages(); });

        autoscroll_.setInterval(16);
        QObject::connect(&autoscroll_, &QTimer::timeout, this, [this] { autoScrollTick(); });
    }

    void load(const QJsonObject& root) {
        QElapsedTimer timer;
        timer.start();
        commands_.clear();
        for (const auto& value : root.value("commands").toArray()) {
            commands_.push_back(parseCommand(value));
        }
        image_occurrences_.clear();
        image_requested_.clear();
        code_sources_.clear();
        title_ = root.value("title").toString(QStringLiteral("Moonmark"));
        settings_ = root.value("settings").toObject();
        metrics_ = root.value("metrics").toObject();

        auto* next = new QTextDocument(this);
        next->setDocumentMargin(0.0);
        next->setDefaultFont(baseCharacterFormat(settings_.value("bodyFontPoints").toDouble(12.75))
                                 .font());
        setDocument(next);
        applyDocumentWidth();

        QTextCursor cursor(next);
        cursor.beginEditBlock();
        const auto error = root.value("error").toString();
        if (!error.isEmpty()) {
            auto block = bodyBlockFormat();
            block.setTopMargin(28);
            cursor.setBlockFormat(block);
            auto format = baseCharacterFormat();
            format.setForeground(QColor(colour::error));
            cursor.insertText(error, format);
        } else {
            std::size_t index = 0;
            buildBlocks(cursor, index, -1, 0);
        }
        cursor.endEditBlock();
        construction_us_ = static_cast<quint64>(timer.nsecsElapsed() / 1000);
        document_construction_count_++;
        QTextCursor start(next);
        start.movePosition(QTextCursor::Start);
        setTextCursor(start);
        verticalScrollBar()->setValue(0);
        QTimer::singleShot(0, this, [this] {
            verticalScrollBar()->setValue(0);
            queueVisibleImages();
        });
    }

    [[nodiscard]] QString title() const { return title_; }
    [[nodiscard]] quint64 constructionMicros() const { return construction_us_; }
    [[nodiscard]] quint64 constructionCount() const { return document_construction_count_; }
    [[nodiscard]] int discoveredImages() const {
        return static_cast<int>(image_occurrences_.size());
    }
    [[nodiscard]] int loadedImages() const {
        return static_cast<int>(std::count_if(image_occurrences_.cbegin(), image_occurrences_.cend(),
                                              [](const auto& item) { return item.loaded; }));
    }
    [[nodiscard]] int failedImageDecodes() const {
        return static_cast<int>(std::count_if(image_occurrences_.cbegin(), image_occurrences_.cend(),
                                              [](const auto& item) {
                                                  return item.requested && item.failed;
                                              }));
    }
    [[nodiscard]] int pendingImageDecodes() const {
        return static_cast<int>(std::count_if(image_occurrences_.cbegin(), image_occurrences_.cend(),
                                              [](const auto& item) {
                                                  return item.requested && !item.loaded && !item.failed;
                                              }));
    }
    [[nodiscard]] QString plainText() const { return document()->toPlainText(); }
    [[nodiscard]] bool testSelectionCopy() {
        auto* previous = snapshotClipboard();
        QKeyEvent select_event(QEvent::KeyPress, Qt::Key_A, Qt::ControlModifier);
        QApplication::sendEvent(this, &select_event);
        QKeyEvent copy_event(QEvent::KeyPress, Qt::Key_C, Qt::ControlModifier);
        QApplication::sendEvent(this, &copy_event);
        const auto copied = QGuiApplication::clipboard()->text();
        QGuiApplication::clipboard()->setMimeData(previous);
        return copied.size() > 40 && !copied.contains(QChar::ObjectReplacementCharacter) &&
               !copied.contains(QChar(0xFDD0)) && !copied.contains(QChar(0xFDD1));
    }

    [[nodiscard]] bool testCodeCopy() {
        if (code_sources_.empty()) return true;
        for (auto block = document()->begin(); block.isValid(); block = block.next()) {
            for (auto fragment = block.begin(); !fragment.atEnd(); ++fragment) {
                const auto part = fragment.fragment();
                if (part.charFormat().anchorHref() != QStringLiteral("moonmark-copy:0")) continue;
                QTextCursor cursor(document());
                cursor.setPosition(part.position() + 1);
                setTextCursor(cursor);
                ensureCursorVisible();
                const auto point = cursorRect(cursor).center();
                auto* previous = snapshotClipboard();
                QMouseEvent press(QEvent::MouseButtonPress, point, viewport()->mapToGlobal(point),
                                  Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                QMouseEvent release(QEvent::MouseButtonRelease, point, viewport()->mapToGlobal(point),
                                    Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
                QApplication::sendEvent(viewport(), &press);
                QApplication::sendEvent(viewport(), &release);
                const bool copied = QGuiApplication::clipboard()->text() == code_sources_.front();
                if (!copied) {
                    std::fprintf(stdout, "COPY_DIAGNOSTIC press=%d cursor=%d selected=%d anchor=%s clipboard_length=%lld expected_length=%lld\n",
                                 press_position_, textCursor().position(), textCursor().hasSelection(),
                                 cursorForPosition(point).charFormat().anchorHref().toUtf8().constData(),
                                 static_cast<long long>(QGuiApplication::clipboard()->text().size()),
                                 static_cast<long long>(code_sources_.front().size()));
                }
                QGuiApplication::clipboard()->setMimeData(previous);
                return copied;
            }
        }
        return false;
    }

    [[nodiscard]] bool testDocumentStyle() const {
        int tables = 0;
        bool right_aligned = false;
        for (auto* frame : document()->rootFrame()->childFrames()) {
            auto* table = qobject_cast<QTextTable*>(frame);
            if (table == nullptr) continue;
            ++tables;
            if (table->format().border() != 0) return false;
            for (int row = 0; row < table->rows(); ++row) {
                for (int column = 0; column < table->columns(); ++column) {
                    const auto cell = table->cellAt(row, column);
                    const auto format = cell.format().toTableCellFormat();
                    if (format.leftBorder() != 0 || format.rightBorder() != 0 ||
                        format.topBorder() != 0) return false;
                    right_aligned |= cell.firstCursorPosition().blockFormat().alignment() ==
                                     Qt::AlignRight;
                    const auto block = cell.firstCursorPosition().block();
                    for (auto it = block.begin(); !it.atEnd(); ++it) {
                        if (it.fragment().charFormat().background().style() != Qt::NoBrush)
                            return false;
                    }
                }
            }
        }
        const auto palette = QApplication::palette();
        for (const auto group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled}) {
            for (const auto role : {QPalette::Accent, QPalette::Highlight, QPalette::Link,
                                    QPalette::Text, QPalette::Button, QPalette::ButtonText}) {
                const auto color = palette.color(group, role);
                if (color.red() != color.green() || color.green() != color.blue()) return false;
            }
        }
        auto* accessible = QAccessible::queryAccessibleInterface(
            const_cast<DocumentView*>(this));
        return tables > 0 && right_aligned && accessible != nullptr &&
               accessible->textInterface() != nullptr && isReadOnly();
    }

    void queueAllImagesForSmoke() {
        for (auto& occurrence : image_occurrences_) {
            if (occurrence.loaded || occurrence.failed || occurrence.requested ||
                image_requested_.contains(occurrence.id)) {
                continue;
            }
            if (api_->queue_image(backend_, occurrence.id, 1600)) {
                image_requested_.emplace(occurrence.id, true);
                for (auto& same : image_occurrences_) {
                    if (same.id == occurrence.id) {
                        same.requested = true;
                    }
                }
            }
        }
        image_poll_.start();
    }

    void changeZoom(int delta) {
        const int previous = zoom_percent_;
        zoom_percent_ = std::clamp(zoom_percent_ + delta, 60, 220);
        if (zoom_percent_ == previous) {
            return;
        }
        const int point_steps = (zoom_percent_ - previous) / 10;
        if (point_steps > 0) {
            zoomIn(point_steps);
        } else {
            zoomOut(-point_steps);
        }
        applyDocumentWidth();
        resizeLoadedImages();
    }

    [[nodiscard]] int zoomPercent() const { return zoom_percent_; }

protected:
    void paintEvent(QPaintEvent* event) override {
        QTextEdit::paintEvent(event);
        QPainter painter(viewport());
        painter.setPen(QPen(QColor(colour::border_strong), 2));
        // Qt retains layout, selection, and accessibility; only quote markers are painted.
        auto block = cursorForPosition(QPoint(0, 0)).block();
        for (; block.isValid(); block = block.next()) {
            QTextCursor cursor(block);
            const auto first = cursorRect(cursor);
            if (first.top() > viewport()->height()) break;
            const int depth = block.blockFormat().intProperty(quote_depth_property);
            if (depth == 0 || block.text().isEmpty()) continue;
            cursor.movePosition(QTextCursor::EndOfBlock);
            auto bottom = cursorRect(cursor).bottom() + 4;
            const auto next = block.next();
            if (next.isValid() &&
                next.blockFormat().intProperty(quote_depth_property) == depth &&
                !next.text().isEmpty()) {
                bottom = cursorRect(QTextCursor(next)).top();
            }
            for (int level = 0; level < depth; ++level) {
                const int x = first.left() - 14 - level * 22;
                painter.drawLine(x, first.top() - 3, x, bottom);
            }
        }
    }

    void resizeEvent(QResizeEvent* event) override {
        QTextEdit::resizeEvent(event);
        applyDocumentWidth();
        resizeLoadedImages();
        QTimer::singleShot(0, this, [this] { queueVisibleImages(); });
    }

    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::MiddleButton) {
            autoscroll_active_ = !autoscroll_active_;
            autoscroll_anchor_ = event->position();
            autoscroll_pointer_ = event->position();
            if (autoscroll_active_) {
                viewport()->setCursor(Qt::SizeVerCursor);
                autoscroll_.start();
            } else {
                stopAutoscroll();
            }
            event->accept();
            return;
        }
        if (autoscroll_active_) {
            stopAutoscroll();
            event->accept();
            return;
        }
        press_position_ = cursorForPosition(event->position().toPoint()).position();
        press_point_ = event->position().toPoint();
        selection_drag_ = false;
        QTextEdit::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        if (autoscroll_active_) {
            autoscroll_pointer_ = event->position();
            event->accept();
            return;
        }
        if ((event->buttons() & Qt::LeftButton) != 0 &&
            (event->position().toPoint() - press_point_).manhattanLength() > 3) {
            selection_drag_ = true;
        }
        QTextEdit::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        QTextEdit::mouseReleaseEvent(event);
        if (event->button() != Qt::LeftButton || selection_drag_ ||
            (event->position().toPoint() - press_point_).manhattanLength() > 3 ||
            event->modifiers().testFlag(Qt::ShiftModifier)) {
            return;
        }
        // Qt may select the link label on mouse release. Detect a click by its
        // gesture/anchor, not by the resulting native selection.
        const auto anchor = anchorAt(event->position().toPoint());
        if (anchor.isEmpty() || anchor != anchorAt(press_point_)) return;
        if (anchor.startsWith(QStringLiteral("moonmark-copy:"))) {
            const auto index = anchor.sliced(QStringLiteral("moonmark-copy:").size()).toInt();
            if (index >= 0 && index < static_cast<int>(code_sources_.size())) {
                QGuiApplication::clipboard()->setText(code_sources_[static_cast<std::size_t>(index)]);
            }
        } else if (anchor.startsWith(QLatin1Char('#'))) {
            scrollToAnchor(QUrl::fromPercentEncoding(anchor.sliced(1).toUtf8()));
        } else {
            const QUrl url(anchor);
            if (url.scheme() == QStringLiteral("http") || url.scheme() == QStringLiteral("https") ||
                url.scheme() == QStringLiteral("mailto")) {
                QDesktopServices::openUrl(url);
            }
        }
    }

    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_Escape && autoscroll_active_) {
            stopAutoscroll();
            event->accept();
            return;
        }
        if (event->matches(QKeySequence::Copy) && textCursor().hasSelection()) {
            auto selected = textCursor().selectedText();
            selected.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
            selected.replace(QChar::LineSeparator, QLatin1Char('\n'));
            selected.remove(QChar::ObjectReplacementCharacter);
            selected.remove(QChar(0xFDD0));
            selected.remove(QChar(0xFDD1));
            selected.remove(QChar(0x200B));
            QGuiApplication::clipboard()->setText(selected);
            event->accept();
            return;
        }
        QTextEdit::keyPressEvent(event);
    }

private:
    void buildBlocks(QTextCursor& cursor, std::size_t& index, int stop_kind, int depth) {
        while (index < commands_.size()) {
            const auto& command = commands_[index];
            if (command.kind == stop_kind) {
                ++index;
                return;
            }
            switch (command.kind) {
            case command_kind::begin_paragraph:
                ++index;
                buildParagraph(cursor, index, false, 0, depth);
                break;
            case command_kind::begin_heading:
                ++index;
                buildParagraph(cursor, index, true, command.level, depth, command.target);
                break;
            case command_kind::begin_list:
                buildList(cursor, index, depth);
                break;
            case command_kind::code_block:
                buildCodeBlock(cursor, command, depth);
                ++index;
                break;
            case command_kind::horizontal_rule:
                buildRule(cursor, depth);
                ++index;
                break;
            case command_kind::begin_quote:
                ++index;
                ++quote_depth_;
                buildBlocks(cursor, index, command_kind::end_quote, depth + 1);
                --quote_depth_;
                cursor.setBlockFormat(bodyBlockFormat());
                break;
            case command_kind::begin_table:
                buildTable(cursor, index, depth);
                break;
            case command_kind::image:
                buildImage(cursor, command, depth);
                ++index;
                break;
            case command_kind::raw_html:
                buildRawHtml(cursor, command, depth);
                ++index;
                break;
            default:
                ++index;
                break;
            }
        }
    }

    void beginBlock(QTextCursor& cursor, QTextBlockFormat format, int depth) {
        if (cursor.position() != 0 && !cursor.block().text().isEmpty()) {
            cursor.insertBlock();
        }
        if (depth > 0) {
            format.setLeftMargin(format.leftMargin() + depth * 22.0);
            format.setRightMargin(format.rightMargin() + 8.0);
        }
        format.setProperty(quote_depth_property, quote_depth_);
        cursor.setBlockFormat(format);
    }

    void buildParagraph(QTextCursor& cursor, std::size_t& index, bool heading, int level, int depth,
                        const QString& anchor = {}) {
        auto block = bodyBlockFormat(settings_.value("lineHeightPercent").toInt(150));
        double points = settings_.value("bodyFontPoints").toDouble(12.75);
        if (heading) {
            static constexpr double scales[] = {1.85, 1.55, 1.34, 1.18, 1.08, 1.0};
            points *= scales[std::clamp(level, 1, 6) - 1];
            block.setTopMargin(level == 1 ? 21.0 : (level == 2 ? 17.0 : 13.0));
            block.setBottomMargin(level <= 2 ? 9.0 : 6.0);
            block.setLineHeight(132, QTextBlockFormat::ProportionalHeight);
        }
        beginBlock(cursor, block, depth);
        if (!anchor.isEmpty()) {
            QTextCharFormat named;
            named.setAnchor(true);
            named.setAnchorNames({anchor});
            cursor.insertText(QString(QChar::ObjectReplacementCharacter), named);
        }
        while (index < commands_.size() && commands_[index].kind != command_kind::end_block) {
            insertInline(cursor, commands_[index], points, heading);
            ++index;
        }
        if (index < commands_.size()) {
            ++index;
        }
        cursor.insertBlock();
    }

    void insertInline(QTextCursor& cursor, const Command& command, double points, bool heading,
                      bool table_context = false) {
        if (command.kind == command_kind::soft_break) {
            cursor.insertText(QStringLiteral(" "), baseCharacterFormat(points));
            return;
        }
        if (command.kind == command_kind::hard_break) {
            cursor.insertText(QString(QChar::LineSeparator), baseCharacterFormat(points));
            return;
        }
        if (command.kind == command_kind::image) {
            buildInlineImage(cursor, command);
            return;
        }
        if (command.kind == command_kind::raw_html) {
            auto raw = baseCharacterFormat(points * 0.92);
            raw.setFontFamilies({QStringLiteral("Cascadia Mono"), QStringLiteral("Consolas")});
            raw.setForeground(QColor(colour::muted));
            cursor.insertText(command.text, raw);
            return;
        }
        if (command.kind != command_kind::text) {
            return;
        }
        auto format = baseCharacterFormat(points);
        if (quote_depth_ > 0) format.setForeground(QColor(colour::secondary));
        if (heading || (command.flags & text_style::strong) != 0) {
            format.setFontWeight(heading ? QFont::DemiBold : QFont::Bold);
        }
        if ((command.flags & text_style::emphasis) != 0) {
            format.setFontItalic(true);
        }
        if ((command.flags & text_style::strike) != 0) {
            format.setFontStrikeOut(true);
        }
        if ((command.flags & text_style::code) != 0) {
            format.setFontFamilies({QStringLiteral("Cascadia Mono"), QStringLiteral("Consolas")});
            format.setFontPointSize(points * 0.9);
            if (!table_context) format.setBackground(QColor(colour::surface));
            format.setForeground(QColor(colour::bright));
        }
        if ((command.flags & text_style::link) != 0) {
            format.setAnchor(true);
            format.setAnchorHref(command.target);
            format.setForeground(QColor(colour::silver));
            format.setFontUnderline(true);
        }
        cursor.insertText(command.text, format);
    }

    void buildList(QTextCursor& cursor, std::size_t& index, int depth) {
        const auto begin = commands_[index++];
        int item_number = static_cast<int>(begin.number);
        while (index < commands_.size() && commands_[index].kind != command_kind::end_list) {
            if (commands_[index].kind != command_kind::begin_item) {
                ++index;
                continue;
            }
            const auto item = commands_[index++];
            QString marker = item.text;
            if ((item.flags & text_style::checked) != 0) {
                marker = QStringLiteral("☑");
            } else if ((item.flags & text_style::unchecked) != 0) {
                marker = QStringLiteral("☐");
            } else if ((begin.flags & text_style::ordered) != 0) {
                marker = QString::number(item_number++) + QStringLiteral(".");
            }

            bool marker_inserted = false;
            while (index < commands_.size() && commands_[index].kind != command_kind::end_item) {
                if (commands_[index].kind == command_kind::begin_paragraph) {
                    ++index;
                    auto block = bodyBlockFormat(settings_.value("lineHeightPercent").toInt(150));
                    block.setLeftMargin((depth + 1) * 24.0);
                    block.setTextIndent(marker_inserted ? 0.0 : -22.0);
                    QTextOption::Tab tab;
                    tab.position = 22;
                    block.setTabPositions({tab});
                    block.setBottomMargin(4.0);
                    beginBlock(cursor, block, 0);
                    auto marker_format = baseCharacterFormat();
                    marker_format.setForeground(QColor(colour::secondary));
                    if (!marker_inserted) cursor.insertText(marker + QLatin1Char('\t'), marker_format);
                    marker_inserted = true;
                    while (index < commands_.size() &&
                           commands_[index].kind != command_kind::end_block) {
                        insertInline(cursor, commands_[index],
                                     settings_.value("bodyFontPoints").toDouble(12.75), false);
                        ++index;
                    }
                    if (index < commands_.size()) {
                        ++index;
                    }
                    cursor.insertBlock();
                } else if (commands_[index].kind == command_kind::begin_list) {
                    buildList(cursor, index, depth + 1);
                } else {
                    buildBlocks(cursor, index, command_kind::end_item, depth + 1);
                }
            }
            if (index < commands_.size() && commands_[index].kind == command_kind::end_item) {
                ++index;
            }
            if (!marker_inserted) {
                auto block = bodyBlockFormat();
                block.setLeftMargin((depth + 1) * 24.0);
                beginBlock(cursor, block, 0);
                cursor.insertText(marker, baseCharacterFormat());
                cursor.insertBlock();
            }
        }
        if (index < commands_.size()) {
            ++index;
        }
    }

    void buildCodeBlock(QTextCursor& cursor, const Command& command, int depth) {
        if (cursor.position() != 0 && !cursor.block().text().isEmpty()) {
            cursor.insertBlock();
        }
        // QTextDocument requires an outer paragraph around frames. Keep the empty
        // structural paragraph tiny rather than giving it a full body-text line.
        auto spacer = bodyBlockFormat();
        spacer.setLineHeight(1, QTextBlockFormat::FixedHeight);
        spacer.setTopMargin(0);
        spacer.setBottomMargin(0);
        cursor.setBlockFormat(spacer);
        QTextFrameFormat frame_format;
        frame_format.setBackground(QColor(colour::surface));
        frame_format.setBorder(0);
        frame_format.setPadding(14.0);
        frame_format.setTopMargin(6.0);
        frame_format.setBottomMargin(13.0);
        frame_format.setLeftMargin(depth * 22.0);
        auto* frame = cursor.insertFrame(frame_format);
        QTextCursor inside(frame);

        const auto code_index = static_cast<int>(code_sources_.size());
        QString source;
        for (const auto& span : command.spans) {
            source += span.toObject().value("text").toString();
        }
        code_sources_.push_back(source);

        auto header_block = bodyBlockFormat(115);
        header_block.setTopMargin(0);
        header_block.setBottomMargin(5);
        inside.setBlockFormat(header_block);
        auto language = baseCharacterFormat(9.5);
        language.setForeground(QColor(colour::muted));
        language.setFontWeight(QFont::Normal);
        inside.insertText(command.extra.isEmpty() ? QStringLiteral("code") : command.extra.toLower(),
                          language);
        inside.insertText(QStringLiteral("   ·   "), language);
        auto copy = language;
        copy.setAnchor(true);
        copy.setAnchorHref(QStringLiteral("moonmark-copy:%1").arg(code_index));
        copy.setForeground(QColor(colour::silver));
        copy.setFontUnderline(false);
        inside.insertText(QStringLiteral("Copy"), copy);
        inside.insertBlock();

        auto separator_block = bodyBlockFormat(100);
        separator_block.setLineHeight(1, QTextBlockFormat::FixedHeight);
        separator_block.setTopMargin(0);
        separator_block.setBottomMargin(8);
        separator_block.setBackground(QColor(colour::border));
        inside.setBlockFormat(separator_block);
        inside.insertText(QString(QChar(0x200B)), baseCharacterFormat(1));
        inside.insertBlock();

        auto code_block = bodyBlockFormat(142);
        code_block.setTopMargin(0);
        code_block.setBottomMargin(0);
        code_block.setNonBreakableLines(true);
        inside.setBlockFormat(code_block);
        for (const auto& value : command.spans) {
            const auto span = value.toObject();
            QTextCharFormat format;
            format.setFontFamilies({QStringLiteral("Cascadia Mono"), QStringLiteral("Consolas")});
            format.setFontPointSize(10.5);
            format.setForeground(QColor(span.value("red").toInt(), span.value("green").toInt(),
                                        span.value("blue").toInt()));
            const int flags = span.value("flags").toInt();
            format.setFontWeight((flags & 1) != 0 ? QFont::DemiBold : QFont::Normal);
            format.setFontItalic((flags & 2) != 0);
            inside.insertText(span.value("text").toString(), format);
        }
        cursor = QTextCursor(frame->lastCursorPosition());
        cursor.movePosition(QTextCursor::End);
        cursor.setBlockFormat(bodyBlockFormat());
        cursor.setCharFormat(baseCharacterFormat());
    }

    void buildRule(QTextCursor& cursor, int depth) {
        auto block = bodyBlockFormat(100);
        block.setLineHeight(1, QTextBlockFormat::FixedHeight);
        block.setTopMargin(10);
        block.setBottomMargin(10);
        block.setLeftMargin(depth * 22.0);
        block.setBackground(QColor(colour::border));
        beginBlock(cursor, block, 0);
        cursor.insertText(QString(QChar(0x200B)), baseCharacterFormat(1));
        cursor.insertBlock();
    }

    void buildTable(QTextCursor& cursor, std::size_t& index, int depth) {
        const auto begin = commands_[index++];
        if (cursor.position() != 0 && !cursor.block().text().isEmpty()) {
            cursor.insertBlock();
        }
        auto spacer = bodyBlockFormat();
        spacer.setLineHeight(1, QTextBlockFormat::FixedHeight);
        spacer.setTopMargin(0);
        spacer.setBottomMargin(0);
        cursor.setBlockFormat(spacer);
        QTextTableFormat table_format;
        table_format.setBorder(0);
        table_format.setBorderCollapse(true);
        table_format.setHeaderRowCount(1);
        table_format.setWidth(QTextLength(QTextLength::PercentageLength, 100));
        table_format.setCellPadding(0);
        table_format.setCellSpacing(0.0);
        table_format.setTopMargin(6.0);
        table_format.setBottomMargin(13.0);
        table_format.setLeftMargin(depth * 22.0);
        int rows = 0;
        for (std::size_t scan = index; scan < commands_.size() &&
                                       commands_[scan].kind != command_kind::end_table;
             ++scan) {
            rows += commands_[scan].kind == command_kind::begin_row ? 1 : 0;
        }
        const int columns = std::max(1, static_cast<int>(begin.number));
        auto* table = cursor.insertTable(std::max(1, rows), columns, table_format);
        int row = 0;
        while (index < commands_.size() && commands_[index].kind != command_kind::end_table) {
            const auto row_command = commands_[index++];
            if (row_command.kind != command_kind::begin_row) {
                continue;
            }
            int column = 0;
            while (index < commands_.size() && commands_[index].kind != command_kind::end_row) {
                const auto cell_command = commands_[index++];
                if (cell_command.kind != command_kind::begin_cell) {
                    continue;
                }
                auto cell = table->cellAt(row, std::min(column, columns - 1));
                auto cell_format = cell.format().toTableCellFormat();
                const bool header = (row_command.flags & text_style::header) != 0;
                cell_format.setTopPadding(6);
                cell_format.setBottomPadding(6);
                cell_format.setLeftPadding(column == 0 ? 0 : 12);
                cell_format.setRightPadding(column == columns - 1 ? 0 : 12);
                if (row < rows - 1) {
                    cell_format.setBottomBorder(1);
                    cell_format.setBottomBorderStyle(QTextFrameFormat::BorderStyle_Solid);
                    cell_format.setBottomBorderBrush(QColor(header ? colour::border_strong :
                                                                     colour::border));
                }
                cell.setFormat(cell_format);
                auto cell_cursor = cell.firstCursorPosition();
                auto cell_block = bodyBlockFormat(125);
                cell_block.setTopMargin(0);
                cell_block.setBottomMargin(0);
                cell_block.setAlignment(cell_command.number == 2 ? Qt::AlignRight :
                                       cell_command.number == 1 ? Qt::AlignHCenter : Qt::AlignLeft);
                cell_cursor.setBlockFormat(cell_block);
                while (index < commands_.size() && commands_[index].kind != command_kind::end_cell) {
                    insertInline(cell_cursor, commands_[index], 11.25,
                                 (row_command.flags & text_style::header) != 0, true);
                    ++index;
                }
                if (index < commands_.size()) {
                    ++index;
                }
                ++column;
            }
            if (index < commands_.size()) {
                ++index;
            }
            ++row;
        }
        if (index < commands_.size()) {
            ++index;
        }
        cursor = QTextCursor(table->lastCursorPosition());
        cursor.movePosition(QTextCursor::End);
        cursor.setBlockFormat(bodyBlockFormat());
        cursor.setCharFormat(baseCharacterFormat());
    }

    void buildImage(QTextCursor& cursor, const Command& command, int depth) {
        auto block = bodyBlockFormat(125);
        block.setTopMargin(6);
        block.setBottomMargin(13);
        beginBlock(cursor, block, depth);
        buildInlineImage(cursor, command);
        cursor.insertBlock();
    }

    void buildInlineImage(QTextCursor& cursor, const Command& command) {
        const auto id = static_cast<std::uint32_t>(command.number);
        const bool allowed = command.flags == 0;
        QString message;
        if (allowed) {
            message = command.text.isEmpty() ? QStringLiteral("Loading image…")
                                             : QStringLiteral("Loading %1…").arg(command.text);
        } else if (command.flags == 1) {
            message = QStringLiteral("Image not found · %1").arg(command.target);
        } else if (command.flags == 2) {
            message = QStringLiteral("Image reference blocked · %1").arg(command.target);
        } else if (command.flags == 3) {
            message = QStringLiteral("Remote image blocked by Moonmark's privacy policy");
        } else if (command.flags == 4) {
            message = QStringLiteral("Unsupported image format · %1").arg(command.target);
        } else {
            message = QStringLiteral("Image could not be displayed");
        }

        const auto resource = QUrl(QStringLiteral("moonmark-image://%1").arg(id));
        document()->addResource(QTextDocument::ImageResource, resource,
                                placeholderImage(message, 900, 72, !allowed));
        QTextImageFormat format;
        format.setName(resource.toString());
        format.setWidth(std::min(900, availableImageWidth()));
        format.setHeight(72);
        format.setToolTip(command.text);
        const int position = cursor.position();
        cursor.insertImage(format);
        image_occurrences_.push_back(
            ImageOccurrence{id, position, false, false, !allowed, 900, 72});
    }

    void buildRawHtml(QTextCursor& cursor, const Command& command, int depth) {
        auto block = bodyBlockFormat(145);
        block.setTopMargin(4);
        block.setBottomMargin(10);
        beginBlock(cursor, block, depth);
        auto format = baseCharacterFormat(10.0);
        format.setFontFamilies({QStringLiteral("Cascadia Mono"), QStringLiteral("Consolas")});
        format.setForeground(QColor(colour::muted));
        cursor.insertText(command.text, format);
        cursor.insertBlock();
    }

    int availableImageWidth() const {
        return std::max(240, viewport()->width() - effectiveSideMargin() * 2 - 8);
    }

    int effectiveSideMargin() const {
        const int base = settings_.value("documentPadding").toInt(48);
        return std::min(base, std::max(20, (viewport()->width() - 400) / 12));
    }

    void applyDocumentWidth() {
        if (document() == nullptr || document()->rootFrame() == nullptr) {
            return;
        }
        auto format = document()->rootFrame()->frameFormat();
        const auto margin = static_cast<qreal>(effectiveSideMargin());
        format.setLeftMargin(margin);
        format.setRightMargin(margin);
        format.setTopMargin(22.0);
        format.setBottomMargin(34.0);
        document()->rootFrame()->setFrameFormat(format);
    }

    void queueVisibleImages() {
        if (backend_ == nullptr || document() == nullptr) {
            return;
        }
        const int prefetch = 900;
        for (auto& occurrence : image_occurrences_) {
            if (occurrence.requested || occurrence.loaded || occurrence.failed) {
                continue;
            }
            QTextCursor cursor(document());
            cursor.setPosition(std::min(occurrence.position, document()->characterCount() - 1));
            const auto rectangle = cursorRect(cursor);
            if (rectangle.bottom() < -prefetch || rectangle.top() > viewport()->height() + prefetch) {
                continue;
            }
            if (image_requested_.contains(occurrence.id)) {
                occurrence.requested = true;
                continue;
            }
            if (api_->queue_image(backend_, occurrence.id,
                                  static_cast<std::uint32_t>(std::max(240, availableImageWidth())))) {
                image_requested_.emplace(occurrence.id, true);
                for (auto& same : image_occurrences_) {
                    if (same.id == occurrence.id) {
                        same.requested = true;
                    }
                }
                image_poll_.start();
            }
        }
    }

    void pollImages() {
        bool received = false;
        while (true) {
            auto result = api_->poll_image(backend_);
            if (result.id == 0) {
                break;
            }
            received = true;
            const auto error = fromBuffer(result.error);
            if (result.error.data != nullptr) {
                api_->buffer_free(result.error);
            }
            const auto resource = QUrl(QStringLiteral("moonmark-image://%1").arg(result.id));
            if (!error.isEmpty() || result.pixels.data == nullptr || result.width == 0 ||
                result.height == 0) {
                document()->addResource(QTextDocument::ImageResource, resource,
                                        placeholderImage(QStringLiteral("Image failed · %1").arg(error),
                                                         900, 72, true));
                for (auto& occurrence : image_occurrences_) {
                    if (occurrence.id == result.id) {
                        occurrence.failed = true;
                    }
                }
            } else {
                const auto width = static_cast<int>(result.width);
                const auto height = static_cast<int>(result.height);
                QImage view(result.pixels.data, width, height, width * 4, QImage::Format_RGBA8888);
                document()->addResource(QTextDocument::ImageResource, resource, view.copy());
                for (auto& occurrence : image_occurrences_) {
                    if (occurrence.id == result.id) {
                        occurrence.loaded = true;
                        occurrence.natural_width = width;
                        occurrence.natural_height = height;
                    }
                }
            }
            if (result.pixels.data != nullptr) {
                api_->buffer_free(result.pixels);
            }
            updateImageFormats(result.id);
        }
        if (received) {
            viewport()->update();
        }
        const bool pending = std::any_of(image_occurrences_.cbegin(), image_occurrences_.cend(),
                                         [](const auto& image) {
                                             return image.requested && !image.loaded && !image.failed;
                                         });
        if (!pending) {
            image_poll_.stop();
        }
    }

    void updateImageFormats(std::uint32_t id) {
        for (const auto& occurrence : image_occurrences_) {
            if (occurrence.id != id) {
                continue;
            }
            QTextCursor cursor(document());
            cursor.setPosition(occurrence.position);
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            auto format = cursor.charFormat().toImageFormat();
            const int width = std::min(occurrence.natural_width, availableImageWidth());
            const double scale = occurrence.natural_width > 0
                                     ? static_cast<double>(width) / occurrence.natural_width
                                     : 1.0;
            format.setWidth(width);
            format.setHeight(std::max(1, static_cast<int>(occurrence.natural_height * scale)));
            cursor.setCharFormat(format);
        }
    }

    void resizeLoadedImages() {
        std::unordered_map<std::uint32_t, bool> changed;
        for (const auto& occurrence : image_occurrences_) {
            if (occurrence.loaded && !changed.contains(occurrence.id)) {
                updateImageFormats(occurrence.id);
                changed.emplace(occurrence.id, true);
            }
        }
    }

    void stopAutoscroll() {
        autoscroll_active_ = false;
        autoscroll_.stop();
        viewport()->unsetCursor();
    }

    void autoScrollTick() {
        const double distance = autoscroll_pointer_.y() - autoscroll_anchor_.y();
        if (std::abs(distance) <= 12.0) {
            return;
        }
        const double speed = std::min(48.0, std::pow((std::abs(distance) - 12.0) / 22.0, 1.35) +
                                                0.5);
        auto* bar = verticalScrollBar();
        bar->setValue(std::clamp(bar->value() + static_cast<int>(std::copysign(speed, distance)),
                                 bar->minimum(), bar->maximum()));
    }

    const MoonmarkApiTable* api_ = nullptr;
    void* backend_ = nullptr;
    std::vector<Command> commands_;
    std::vector<ImageOccurrence> image_occurrences_;
    std::unordered_map<std::uint32_t, bool> image_requested_;
    std::vector<QString> code_sources_;
    QJsonObject settings_;
    QJsonObject metrics_;
    QString title_ = QStringLiteral("Moonmark");
    QTimer image_poll_;
    QTimer autoscroll_;
    bool autoscroll_active_ = false;
    QPointF autoscroll_anchor_;
    QPointF autoscroll_pointer_;
    int quote_depth_ = 0;
    int press_position_ = 0;
    QPoint press_point_;
    bool selection_drag_ = false;
    int zoom_percent_ = 100;
    quint64 construction_us_ = 0;
    quint64 document_construction_count_ = 0;
};

class MoonmarkWindow final : public QWidget {
public:
    explicit MoonmarkWindow(const MoonmarkApiTable* api) : api_(api) {
        backend_ = api_->backend_new();
        window_state_ = api_->window_state_new();
        setObjectName(QStringLiteral("moonmarkWindow"));
        setWindowTitle(QStringLiteral("Moonmark"));
        setWindowIcon(applicationIcon());
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint |
                       Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
        setAttribute(Qt::WA_NativeWindow);
        setMinimumSize(720, 480);
        resize(1280, 820);
        setAcceptDrops(true);
        buildUi();
        setupWatcher();
        updateStatus();
    }

    ~MoonmarkWindow() override {
        api_->window_state_free(window_state_);
        api_->backend_free(backend_);
    }

    bool openDocument(const QString& path) {
        const QFileInfo file(path);
        if (!file.exists()) {
            return false;
        }
        const auto utf8 = file.canonicalFilePath().toUtf8();
        const auto buffer = api_->open_document(backend_,
                                                reinterpret_cast<const std::uint8_t*>(utf8.constData()),
                                                static_cast<std::size_t>(utf8.size()));
        const auto root = jsonFromBuffer(buffer);
        api_->buffer_free(buffer);
        document_->load(root);
        current_path_ = file.canonicalFilePath();
        title_label_->setFullText(file.fileName());
        title_label_->setToolTip(current_path_);
        setWindowTitle(QStringLiteral("%1 — Moonmark").arg(file.fileName()));
        stack_->setCurrentWidget(document_);
        document_->setFocus(Qt::OtherFocusReason);
        reload_action_->setEnabled(true);
        if (!fullscreen_) {
            document_actions_->show();
        }
        QSettings settings;
        settings.setValue(QStringLiteral("lastOpenDirectory"), file.absolutePath());
        watchCurrentFile();
        updateStatus();
        return root.value("error").toString().isEmpty();
    }

    void runSmoke(const QString& mode) {
        if (mode == QStringLiteral("icon")) {
            QTimer::singleShot(50, this, [] {
                const auto icon = QGuiApplication::windowIcon();
                const auto sizes = icon.availableSizes();
                const bool ok = !icon.isNull() && !sizes.isEmpty();
                std::fprintf(stdout, "MOONMARK_SMOKE icon=%s sizes=%lld\n",
                             ok ? "ok" : "failed", static_cast<long long>(sizes.size()));
                std::fflush(stdout);
                QCoreApplication::exit(ok ? 0 : 11);
            });
            return;
        }
        if (mode == QStringLiteral("snapshot")) {
            const auto width = qEnvironmentVariableIntValue("MOONMARK_SNAPSHOT_WIDTH");
            const auto height = qEnvironmentVariableIntValue("MOONMARK_SNAPSHOT_HEIGHT");
            if (width > 0 && height > 0) resize(width, height);
            QTimer::singleShot(500, this, [this] {
                const auto scroll = qEnvironmentVariableIntValue("MOONMARK_SNAPSHOT_SCROLL");
                if (scroll > 0) document_->verticalScrollBar()->setValue(scroll);
                if (qEnvironmentVariableIsSet("MOONMARK_SNAPSHOT_SELECTION")) document_->selectAll();
                if (qEnvironmentVariableIsSet("MOONMARK_SNAPSHOT_MENU")) {
                    auto* menu = findChild<QMenu*>(QStringLiteral("documentMenu"));
                    menu->popup(mapToGlobal(QPoint(this->width() - 300, 40)));
                    menu->setActiveAction(menu->actions().first());
                }
                QTimer::singleShot(300, this, [this] {
                    auto name = qEnvironmentVariable("MOONMARK_SNAPSHOT_NAME", "moonmark-ui");
                    for (auto& character : name) {
                        if (!character.isLetterOrNumber() && character != QLatin1Char('-'))
                            character = QLatin1Char('_');
                    }
                    QDir::current().mkpath(QStringLiteral("target/visual-dev3"));
                    const auto output = QDir::current().absoluteFilePath(
                        QStringLiteral("target/visual-dev3/%1.png").arg(name));
                    const auto pixels = qEnvironmentVariableIsSet("MOONMARK_SNAPSHOT_MENU")
                        ? findChild<QMenu*>(QStringLiteral("documentMenu"))->grab() : grab();
                    const bool saved = pixels.save(output, "PNG");
                    std::fprintf(stdout, "MOONMARK_SMOKE snapshot=%s path=%s\n",
                                 saved ? "ok" : "failed", output.toUtf8().constData());
                    std::fflush(stdout);
                    QCoreApplication::exit(saved ? 0 : 10);
                });
            });
            return;
        }
        if (mode == QStringLiteral("style")) {
            QTimer::singleShot(180, this, [this] {
                const bool ok = document_->testDocumentStyle() && document_->testSelectionCopy();
                std::fprintf(stdout, "MOONMARK_SMOKE document_style=%s\n", ok ? "ok" : "failed");
                std::fflush(stdout);
                QCoreApplication::exit(ok ? 0 : 11);
            });
            return;
        }
        if (mode == QStringLiteral("render")) {
            QTimer::singleShot(180, this, [this] {
                const auto counters = api_->backend_counters(backend_);
                const bool selection_copy_ok = document_->testSelectionCopy();
                const bool code_copy_ok = document_->testCodeCopy();
                const bool ok = !current_path_.isEmpty() && document_->plainText().size() > 40 &&
                                counters.parse_count == 1 && counters.load_count == 1 &&
                                document_->constructionCount() == 1 && selection_copy_ok && code_copy_ok;
                std::fprintf(stdout,
                             "MOONMARK_SMOKE render=%s chars=%lld construction_us=%llu parse=%llu load=%llu selection_copy=%s code_copy=%s\n",
                             ok ? "ok" : "failed",
                             static_cast<long long>(document_->plainText().size()),
                             static_cast<unsigned long long>(document_->constructionMicros()),
                             static_cast<unsigned long long>(counters.parse_count),
                             static_cast<unsigned long long>(counters.load_count),
                             selection_copy_ok ? "ok" : "failed", code_copy_ok ? "ok" : "failed");
                std::fflush(stdout);
                QCoreApplication::exit(ok ? 0 : 4);
            });
            return;
        }
        if (mode == QStringLiteral("layout")) {
            QTimer::singleShot(300, this, [this] {
                const auto before = api_->backend_counters(backend_);
                const auto constructions = document_->constructionCount();
                resize(width() + 113, height() + 67);
                document_->changeZoom(10);
                document_->changeZoom(-10);
                showMaximized();
                QTimer::singleShot(80, this, [this, before, constructions] {
                    enterFullscreen();
                    QTimer::singleShot(80, this, [this, before, constructions] {
                        leaveFullscreen();
                        QTimer::singleShot(100, this, [this, before, constructions] {
                            const auto after = api_->backend_counters(backend_);
                            const bool ok = before.parse_count == after.parse_count &&
                                            before.load_count == after.load_count &&
                                            before.image_request_count == after.image_request_count &&
                                            constructions == document_->constructionCount() &&
                                            isMaximized();
                            std::fprintf(stdout,
                                         "MOONMARK_SMOKE layout=%s parse_delta=%lld load_delta=%lld construction_delta=%lld image_request_delta=%lld restored=%s\n",
                                         ok ? "ok" : "failed",
                                         static_cast<long long>(after.parse_count - before.parse_count),
                                         static_cast<long long>(after.load_count - before.load_count),
                                         static_cast<long long>(document_->constructionCount() -
                                                                constructions),
                                         static_cast<long long>(after.image_request_count -
                                                                before.image_request_count),
                                         isMaximized() ? "maximized" : "normal");
                            std::fflush(stdout);
                            QCoreApplication::exit(ok ? 0 : 5);
                        });
                    });
                });
            });
            return;
        }
        if (mode == QStringLiteral("maximize")) {
            QTimer::singleShot(300, this, [this] {
                const auto before = api_->backend_counters(backend_);
                const auto constructions = document_->constructionCount();
                const auto normal_geometry = geometry();
                showMaximized();
                QTimer::singleShot(100, this, [this, before, constructions, normal_geometry] {
                    const bool maximized = isMaximized();
                    showNormal();
                    QTimer::singleShot(100, this,
                                       [this, before, constructions, normal_geometry, maximized] {
                        const auto after = api_->backend_counters(backend_);
                        const bool geometry_restored = geometry() == normal_geometry;
                        const bool ok = maximized && !isMaximized() && geometry_restored &&
                                        before.parse_count == after.parse_count &&
                                        before.load_count == after.load_count &&
                                        before.image_request_count == after.image_request_count &&
                                        constructions == document_->constructionCount();
                        std::fprintf(stdout,
                                     "MOONMARK_SMOKE maximize=%s parse_delta=%lld load_delta=%lld construction_delta=%lld image_request_delta=%lld geometry=%s\n",
                                     ok ? "ok" : "failed",
                                     static_cast<long long>(after.parse_count - before.parse_count),
                                     static_cast<long long>(after.load_count - before.load_count),
                                     static_cast<long long>(document_->constructionCount() -
                                                            constructions),
                                     static_cast<long long>(after.image_request_count -
                                                            before.image_request_count),
                                     geometry_restored ? "restored" : "changed");
                        std::fflush(stdout);
                        QCoreApplication::exit(ok ? 0 : 9);
                    });
                });
            });
            return;
        }
        if (mode == QStringLiteral("layout-normal")) {
            QTimer::singleShot(300, this, [this] {
                const auto before = api_->backend_counters(backend_);
                const auto constructions = document_->constructionCount();
                const auto normal_geometry = geometry();
                enterFullscreen();
                QTimer::singleShot(80, this, [this, before, constructions, normal_geometry] {
                    leaveFullscreen();
                    QTimer::singleShot(100, this,
                                       [this, before, constructions, normal_geometry] {
                        const auto after = api_->backend_counters(backend_);
                        const bool geometry_restored = geometry() == normal_geometry;
                        const bool ok = before.parse_count == after.parse_count &&
                                        before.load_count == after.load_count &&
                                        before.image_request_count == after.image_request_count &&
                                        constructions == document_->constructionCount() &&
                                        !isMaximized() && geometry_restored;
                        std::fprintf(stdout,
                                     "MOONMARK_SMOKE layout_normal=%s parse_delta=%lld load_delta=%lld construction_delta=%lld image_request_delta=%lld restored=%s geometry=%s\n",
                                     ok ? "ok" : "failed",
                                     static_cast<long long>(after.parse_count - before.parse_count),
                                     static_cast<long long>(after.load_count - before.load_count),
                                     static_cast<long long>(document_->constructionCount() -
                                                            constructions),
                                     static_cast<long long>(after.image_request_count -
                                                            before.image_request_count),
                                     isMaximized() ? "maximized" : "normal",
                                     geometry_restored ? "restored" : "changed");
                        std::fflush(stdout);
                        QCoreApplication::exit(ok ? 0 : 7);
                    });
                });
            });
            return;
        }
        if (mode == QStringLiteral("watcher")) {
            const auto before = api_->backend_counters(backend_);
            QTimer::singleShot(100, this, [this, before] {
                QFile file(current_path_);
                const bool appended = file.open(QIODevice::Append | QIODevice::Text) &&
                                      file.write("\nMoonmark watcher smoke update.\n") > 0;
                file.close();
                QTimer::singleShot(700, this, [this, before, appended] {
                    const auto after = api_->backend_counters(backend_);
                    const bool ok = appended && after.load_count > before.load_count &&
                                    after.parse_count > before.parse_count;
                    std::fprintf(stdout,
                                 "MOONMARK_SMOKE watcher=%s load_delta=%lld parse_delta=%lld\n",
                                 ok ? "ok" : "failed",
                                 static_cast<long long>(after.load_count - before.load_count),
                                 static_cast<long long>(after.parse_count - before.parse_count));
                    std::fflush(stdout);
                    QCoreApplication::exit(ok ? 0 : 8);
                });
            });
            return;
        }
        if (mode == QStringLiteral("images")) {
            document_->queueAllImagesForSmoke();
            auto* deadline = new QTimer(this);
            deadline->setInterval(25);
            auto* elapsed = new QElapsedTimer;
            elapsed->start();
            QObject::connect(deadline, &QTimer::timeout, this, [this, deadline, elapsed] {
                const bool finished = document_->pendingImageDecodes() == 0;
                const bool timed_out = elapsed->elapsed() > 20000;
                if (!finished && !timed_out) {
                    return;
                }
                const bool ok = finished && document_->failedImageDecodes() == 0 &&
                                document_->loadedImages() > 0;
                const auto counters = api_->backend_counters(backend_);
                std::fprintf(stdout,
                             "MOONMARK_SMOKE images=%s discovered=%d loaded=%d failed=%d pending=%d requests=%llu cache_bytes=%llu elapsed_ms=%lld\n",
                             ok ? "ok" : "failed", document_->discoveredImages(),
                             document_->loadedImages(), document_->failedImageDecodes(),
                             document_->pendingImageDecodes(),
                             static_cast<unsigned long long>(counters.image_request_count),
                             static_cast<unsigned long long>(counters.image_cache_bytes),
                             static_cast<long long>(elapsed->elapsed()));
                std::fflush(stdout);
                deadline->stop();
                delete elapsed;
                QCoreApplication::exit(ok ? 0 : 6);
            });
            deadline->start();
        }
    }

protected:
    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_F11) {
            toggleFullscreen();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_Escape && fullscreen_) {
            leaveFullscreen();
            event->accept();
            return;
        }
        if (event->matches(QKeySequence::Open)) {
            chooseDocument();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_F5) {
            reloadDocument();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_F12) {
            diagnostics_ = !diagnostics_;
            updateStatus();
            event->accept();
            return;
        }
        QWidget::keyPressEvent(event);
    }

    void dragEnterEvent(QDragEnterEvent* event) override {
        if (event->mimeData()->hasUrls()) {
            event->acceptProposedAction();
        }
    }

    void dropEvent(QDropEvent* event) override {
        for (const auto& url : event->mimeData()->urls()) {
            if (url.isLocalFile() && openDocument(url.toLocalFile())) {
                event->acceptProposedAction();
                return;
            }
        }
    }

    void changeEvent(QEvent* event) override {
        QWidget::changeEvent(event);
        if (event->type() == QEvent::WindowStateChange && !fullscreen_) {
            api_->window_set_mode(window_state_, isMaximized() ? 1 : 0);
            maximize_->setMaximized(isMaximized());
        }
    }

#ifdef _WIN32
    bool nativeEvent(const QByteArray& event_type, void* message, qintptr* result) override {
        Q_UNUSED(event_type);
        auto* native = static_cast<MSG*>(message);
        if (native->message == WM_NCHITTEST && !fullscreen_) {
            const auto x = GET_X_LPARAM(native->lParam);
            const auto y = GET_Y_LPARAM(native->lParam);
            RECT rectangle{};
            GetWindowRect(reinterpret_cast<HWND>(winId()), &rectangle);
            const int border = std::max(5, static_cast<int>(6 * devicePixelRatioF()));
            const bool left = x < rectangle.left + border;
            const bool right = x >= rectangle.right - border;
            const bool top = y < rectangle.top + border;
            const bool bottom = y >= rectangle.bottom - border;
            if (top && left) *result = HTTOPLEFT;
            else if (top && right) *result = HTTOPRIGHT;
            else if (bottom && left) *result = HTBOTTOMLEFT;
            else if (bottom && right) *result = HTBOTTOMRIGHT;
            else if (left) *result = HTLEFT;
            else if (right) *result = HTRIGHT;
            else if (top) *result = HTTOP;
            else if (bottom) *result = HTBOTTOM;
            else return QWidget::nativeEvent(event_type, message, result);
            return true;
        }
        if (native->message == WM_SYSKEYDOWN && native->wParam == VK_SPACE) {
            SendMessageW(reinterpret_cast<HWND>(winId()), WM_SYSCOMMAND, SC_KEYMENU, VK_SPACE);
            return true;
        }
        return QWidget::nativeEvent(event_type, message, result);
    }
#endif

private:
    void buildUi() {
        auto* root = new QVBoxLayout(this);
        root->setContentsMargins(0, 0, 0, 0);
        root->setSpacing(0);

        title_bar_ = new moonmark::qt::MoonTitleBar(this);
        auto* title_layout = new QHBoxLayout(title_bar_);
        title_layout->setContentsMargins(14, 0, 0, 0);
        title_layout->setSpacing(9);

        auto* symbol = new QLabel;
        symbol->setPixmap(QApplication::windowIcon().pixmap(18, 18));
        symbol->setFixedSize(20, 20);
        symbol->setAccessibleName(QStringLiteral("Moonmark"));
        symbol->setAttribute(Qt::WA_TransparentForMouseEvents);
        title_layout->addWidget(symbol);

        title_label_ = new ElidingLabel(QStringLiteral("Moonmark"));
        title_label_->setObjectName(QStringLiteral("documentTitle"));
        title_label_->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        title_label_->setMinimumWidth(60);
        title_label_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        title_label_->setAttribute(Qt::WA_TransparentForMouseEvents);
        title_layout->addWidget(title_label_, 1);

        document_actions_ = new QWidget;
        auto* actions = new QHBoxLayout(document_actions_);
        actions->setContentsMargins(0, 0, 6, 0);
        actions->setSpacing(2);
        open_ = new MoonButton(QStringLiteral("Open"));
        open_->setAccessibleName(QStringLiteral("Open Markdown file"));
        open_->setToolTip(QStringLiteral("Open Markdown file (Ctrl+O)"));
        QObject::connect(open_, &QPushButton::clicked, this, [this] { chooseDocument(); });
        actions->addWidget(open_);
        actions->addSpacing(10);
        zoom_out_ = new MoonButton(QStringLiteral("−"));
        zoom_out_->setFixedWidth(28);
        zoom_out_->setAccessibleName(QStringLiteral("Zoom out"));
        zoom_label_ = new QLabel(QStringLiteral("100%"));
        zoom_label_->setObjectName(QStringLiteral("zoomValue"));
        zoom_label_->setAlignment(Qt::AlignCenter);
        zoom_label_->setFixedWidth(40);
        zoom_in_ = new MoonButton(QStringLiteral("+"));
        zoom_in_->setFixedWidth(28);
        zoom_in_->setAccessibleName(QStringLiteral("Zoom in"));
        QObject::connect(zoom_out_, &QPushButton::clicked, this, [this] { changeZoom(-10); });
        QObject::connect(zoom_in_, &QPushButton::clicked, this, [this] { changeZoom(10); });
        actions->addWidget(zoom_out_);
        actions->addWidget(zoom_label_);
        actions->addWidget(zoom_in_);
        auto* more = new MoonButton(QStringLiteral("⋯"));
        more->setAccessibleName(QStringLiteral("Document actions"));
        more->setToolTip(QStringLiteral("Document actions"));
        more->setFixedWidth(32);
        auto* menu = new QMenu(more);
        menu->setObjectName(QStringLiteral("documentMenu"));
        reload_action_ = menu->addAction(QStringLiteral("Reload\tF5"), this,
                                         [this] { reloadDocument(); });
        menu->addAction(QStringLiteral("Reset zoom"), this,
                        [this] { changeZoom(100 - document_->zoomPercent()); });
        menu->addSeparator();
        menu->addAction(QStringLiteral("Fullscreen\tF11"), this,
                        [this] { toggleFullscreen(); });
        menu->addAction(QStringLiteral("Diagnostics\tF12"), this, [this] {
            diagnostics_ = !diagnostics_;
            updateStatus();
        });
        QObject::connect(more, &QPushButton::clicked, this, [more, menu] {
            menu->popup(more->mapToGlobal(QPoint(more->width() - menu->sizeHint().width(),
                                                more->height() + 4)));
        });
        actions->addSpacing(4);
        actions->addWidget(more);
        title_layout->addWidget(document_actions_);
        document_actions_->hide();

        using Caption = moonmark::qt::CaptionButton;
        minimize_ = new Caption(Caption::Action::Minimize);
        maximize_ = new Caption(Caption::Action::Maximize);
        close_ = new Caption(Caption::Action::Close);
        QObject::connect(minimize_, &QAbstractButton::clicked, this, [this] { showMinimized(); });
        QObject::connect(maximize_, &QAbstractButton::clicked, this,
                         [this] { isMaximized() ? showNormal() : showMaximized(); });
        QObject::connect(close_, &QAbstractButton::clicked, this, &QWidget::close);
        auto* captions = new QHBoxLayout;
        captions->setContentsMargins(0, 0, 0, 0);
        captions->setSpacing(0);
        captions->addWidget(minimize_);
        captions->addWidget(maximize_);
        captions->addWidget(close_);
        title_layout->addLayout(captions);
        root->addWidget(title_bar_);

        stack_ = new QStackedWidget;
        stack_->setObjectName(QStringLiteral("documentStack"));
        auto* empty = new QWidget;
        auto* empty_layout = new QVBoxLayout(empty);
        empty_layout->addStretch(2);
        auto* prompt = new QWidget;
        auto* prompt_layout = new QVBoxLayout(prompt);
        prompt_layout->setContentsMargins(0, 0, 0, 0);
        prompt_layout->setSpacing(10);
        auto* identity = new QHBoxLayout;
        identity->setSpacing(12);
        auto* empty_symbol = new QLabel;
        empty_symbol->setPixmap(QApplication::windowIcon().pixmap(32, 32));
        auto* empty_title = new QLabel(QStringLiteral("Moonmark"));
        empty_title->setObjectName(QStringLiteral("emptyTitle"));
        identity->addWidget(empty_symbol);
        identity->addWidget(empty_title);
        identity->addStretch();
        prompt_layout->addLayout(identity);
        auto* empty_hint = new QLabel(QStringLiteral("Drop a Markdown file here to read it."));
        empty_hint->setObjectName(QStringLiteral("emptyHint"));
        prompt_layout->addWidget(empty_hint);
        auto* empty_open = new MoonButton(QStringLiteral("Open Markdown file"));
        empty_open->setObjectName(QStringLiteral("emptyOpen"));
        empty_open->setAccessibleName(QStringLiteral("Open Markdown file"));
        empty_open->setToolTip(QStringLiteral("Ctrl+O"));
        QObject::connect(empty_open, &QPushButton::clicked, this, [this] { chooseDocument(); });
        auto* open_row = new QHBoxLayout;
        open_row->setContentsMargins(0, 4, 0, 0);
        open_row->addWidget(empty_open);
        auto* shortcut = new QLabel(QStringLiteral("Ctrl+O"));
        shortcut->setObjectName(QStringLiteral("emptyHint"));
        open_row->addSpacing(8);
        open_row->addWidget(shortcut);
        open_row->addStretch();
        prompt_layout->addLayout(open_row);
        empty_open->setFocus(Qt::OtherFocusReason);
        empty_layout->addWidget(prompt, 0, Qt::AlignHCenter);
        empty_layout->addStretch(3);
        stack_->addWidget(empty);

        document_ = new DocumentView(api_, backend_);
        stack_->addWidget(document_);
        root->addWidget(stack_, 1);

        diagnostics_bar_ = new QLabel;
        diagnostics_bar_->setObjectName(QStringLiteral("diagnosticsBar"));
        diagnostics_bar_->setFixedHeight(26);
        diagnostics_bar_->setContentsMargins(14, 0, 14, 0);
        diagnostics_bar_->hide();
        root->addWidget(diagnostics_bar_);
    }


    void setupWatcher() {
        watcher_ = new QFileSystemWatcher(this);
        reload_delay_.setSingleShot(true);
        reload_delay_.setInterval(180);
        QObject::connect(watcher_, &QFileSystemWatcher::fileChanged, this, [this] {
            reload_delay_.start();
        });
        QObject::connect(&reload_delay_, &QTimer::timeout, this, [this] {
            if (!current_path_.isEmpty() && QFileInfo::exists(current_path_)) {
                openDocument(current_path_);
            }
        });
    }

    void watchCurrentFile() {
        const auto paths = watcher_->files();
        if (!paths.isEmpty()) {
            watcher_->removePaths(paths);
        }
        if (!current_path_.isEmpty()) {
            watcher_->addPath(current_path_);
        }
    }

    void chooseDocument() {
        QSettings settings;
        auto initial = settings.value(QStringLiteral("lastOpenDirectory")).toString();
        if (initial.isEmpty() || !QFileInfo(initial).isDir()) {
            initial = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        }
        const auto path = QFileDialog::getOpenFileName(
            this, QStringLiteral("Open Markdown file"), initial,
            QStringLiteral("Markdown files (*.md *.markdown *.mdown *.mkd);;All files (*)"));
        if (!path.isEmpty()) {
            openDocument(path);
        }
    }

    void reloadDocument() {
        if (!current_path_.isEmpty()) {
            openDocument(current_path_);
        }
    }

    void changeZoom(int delta) {
        document_->changeZoom(delta);
        zoom_label_->setText(QStringLiteral("%1%").arg(document_->zoomPercent()));
        updateStatus();
    }

    void toggleFullscreen() {
        fullscreen_ ? leaveFullscreen() : enterFullscreen();
    }

    void enterFullscreen() {
        pre_fullscreen_maximized_ = isMaximized();
        api_->window_set_mode(window_state_, pre_fullscreen_maximized_ ? 1 : 0);
        api_->window_enter_fullscreen(window_state_);
        fullscreen_ = true;
        title_bar_->hide();
        document_actions_->hide();
        diagnostics_bar_->hide();
        showFullScreen();
    }

    void leaveFullscreen() {
        const auto restored = api_->window_leave_fullscreen(window_state_);
        fullscreen_ = false;
        title_bar_->show();
        document_actions_->setVisible(!current_path_.isEmpty());
        diagnostics_bar_->setVisible(diagnostics_);
        if (restored == 1 || pre_fullscreen_maximized_) {
            showMaximized();
        } else {
            showNormal();
        }
    }

    void updateStatus() {
        const auto counters = api_->backend_counters(backend_);
        if (diagnostics_) {
            diagnostics_bar_->setText(
                QStringLiteral("%1/%2 images · revision/load %3 · parsed %4× · document %5 µs · cache %6 MiB")
                    .arg(document_->loadedImages())
                    .arg(document_->discoveredImages())
                    .arg(counters.load_count)
                    .arg(counters.parse_count)
                    .arg(document_->constructionMicros())
                    .arg(static_cast<double>(counters.image_cache_bytes) / (1024.0 * 1024.0), 0,
                         'f', 1));
        }
        diagnostics_bar_->setVisible(diagnostics_ && !fullscreen_);
    }

    const MoonmarkApiTable* api_ = nullptr;
    void* backend_ = nullptr;
    void* window_state_ = nullptr;
    moonmark::qt::MoonTitleBar* title_bar_ = nullptr;
    QWidget* document_actions_ = nullptr;
    QStackedWidget* stack_ = nullptr;
    DocumentView* document_ = nullptr;
    MoonButton* open_ = nullptr;
    QAction* reload_action_ = nullptr;
    MoonButton* zoom_out_ = nullptr;
    MoonButton* zoom_in_ = nullptr;
    moonmark::qt::CaptionButton* minimize_ = nullptr;
    moonmark::qt::CaptionButton* maximize_ = nullptr;
    moonmark::qt::CaptionButton* close_ = nullptr;
    QLabel* zoom_label_ = nullptr;
    ElidingLabel* title_label_ = nullptr;
    QLabel* diagnostics_bar_ = nullptr;
    QFileSystemWatcher* watcher_ = nullptr;
    QTimer reload_delay_;
    QString current_path_;
    bool fullscreen_ = false;
    bool pre_fullscreen_maximized_ = false;
    bool diagnostics_ = false;
};

void applyMoonmarkStyle(QApplication& application) {
    moonmark::style::apply(application);
}

} // namespace

extern "C" int moonmark_qt_run(int argc, const char* const* argv, const MoonmarkApiTable* api) {
    if (api == nullptr || api->version != 2) {
        return 2;
    }
    QCoreApplication::setOrganizationName(QStringLiteral("Moonmark"));
    QCoreApplication::setApplicationName(QStringLiteral("Moonmark"));
    QCoreApplication::setApplicationVersion(QStringLiteral(MOONMARK_PRODUCT_VERSION));

    std::vector<QByteArray> argument_storage;
    std::vector<char*> qt_arguments;
    argument_storage.reserve(static_cast<std::size_t>(argc));
    qt_arguments.reserve(static_cast<std::size_t>(argc));
    for (int index = 0; index < argc; ++index) {
        argument_storage.emplace_back(argv[index]);
    }
    for (auto& argument : argument_storage) {
        qt_arguments.push_back(argument.data());
    }
    int qt_argc = argc;
    QApplication application(qt_argc, qt_arguments.data());
    application.setWindowIcon(applicationIcon());
    applyMoonmarkStyle(application);
    // Smoke tests must not update the user's application settings.
    for (const auto& argument : argument_storage) {
        if (argument.startsWith("--smoke-")) {
            QSettings::setDefaultFormat(QSettings::IniFormat);
            QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                              QDir::current().absoluteFilePath("target/native-settings"));
            break;
        }
    }
    MoonmarkWindow window(api);
    window.show();
    QString smoke_mode;
    QString document_path;
    for (int index = 1; index < qt_argc; ++index) {
        const auto argument = QString::fromLocal8Bit(qt_arguments[static_cast<std::size_t>(index)]);
        if (argument == QStringLiteral("--smoke-render")) {
            smoke_mode = QStringLiteral("render");
        } else if (argument == QStringLiteral("--smoke-style")) {
            smoke_mode = QStringLiteral("style");
        } else if (argument == QStringLiteral("--smoke-icon")) {
            smoke_mode = QStringLiteral("icon");
        } else if (argument == QStringLiteral("--smoke-snapshot")) {
            smoke_mode = QStringLiteral("snapshot");
        } else if (argument == QStringLiteral("--smoke-layout")) {
            smoke_mode = QStringLiteral("layout");
        } else if (argument == QStringLiteral("--smoke-maximize")) {
            smoke_mode = QStringLiteral("maximize");
        } else if (argument == QStringLiteral("--smoke-layout-normal")) {
            smoke_mode = QStringLiteral("layout-normal");
        } else if (argument == QStringLiteral("--smoke-watcher")) {
            smoke_mode = QStringLiteral("watcher");
        } else if (argument == QStringLiteral("--smoke-images")) {
            smoke_mode = QStringLiteral("images");
        } else if (!argument.startsWith(QLatin1Char('-')) && QFileInfo::exists(argument)) {
            document_path = argument;
        }
    }
    if (!document_path.isEmpty()) {
        window.openDocument(document_path);
    }
    if (!smoke_mode.isEmpty()) {
        if (document_path.isEmpty() && smoke_mode != QStringLiteral("snapshot") &&
            smoke_mode != QStringLiteral("icon")) {
            return 3;
        }
        window.runSmoke(smoke_mode);
    }
    return application.exec();
}
