#include "document_sidebar.h"
#include "moon_style.h"

#include <QApplication>
#include <QBoxLayout>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <vector>

namespace moonmark::qt {
DocumentSidebar::DocumentSidebar(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("documentSidebar"));
    setAccessibleName(QStringLiteral("Document navigation"));
    setAttribute(Qt::WA_StyledBackground);
    setFixedWidth(style::metric::sidebar_width);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(18, 24, 18, 18);
    layout->setSpacing(8);
    auto* identity = new QHBoxLayout;
    auto* symbol = new QLabel;
    symbol->setPixmap(QApplication::windowIcon().pixmap(24, 24));
    auto* name = new QLabel(QStringLiteral("Moonmark"));
    name->setObjectName(QStringLiteral("sidebarIdentity"));
    identity->addWidget(symbol);
    identity->addSpacing(6);
    identity->addWidget(name);
    identity->addStretch();
    layout->addLayout(identity);
    layout->addSpacing(24);
    auto* open_button = new QPushButton(QStringLiteral("Open document"));
    open_button->setMinimumHeight(36);
    open_button->setToolTip(QStringLiteral("Open Markdown file (Ctrl+O)"));
    connect(open_button, &QPushButton::clicked, this, [this] { if (open) open(); });
    layout->addWidget(open_button);
    reload_ = new QPushButton(QStringLiteral("Reload"));
    reload_->setMinimumHeight(36);
    reload_->setEnabled(false);
    reload_->setToolTip(QStringLiteral("Reload current document (F5)"));
    connect(reload_, &QPushButton::clicked, this, [this] { if (reload) reload(); });
    layout->addWidget(reload_);
    layout->addSpacing(28);
    auto* label = new QLabel(QStringLiteral("CURRENT DOCUMENT"));
    label->setObjectName(QStringLiteral("sidebarSection"));
    layout->addWidget(label);
    filename_ = new QLabel(QStringLiteral("No document open"));
    filename_->setObjectName(QStringLiteral("sidebarFilename"));
    filename_->setWordWrap(true);
    filename_->setTextFormat(Qt::PlainText);
    layout->addWidget(filename_);
    layout->addSpacing(20);
    auto* outline_label = new QLabel(QStringLiteral("OUTLINE"));
    outline_label->setObjectName(QStringLiteral("sidebarSection"));
    layout->addWidget(outline_label);
    outline_ = new QTreeWidget;
    outline_->setAccessibleName(QStringLiteral("Document heading outline"));
    outline_->setHeaderHidden(true);
    outline_->setRootIsDecorated(false);
    outline_->setIndentation(12);
    outline_->setUniformRowHeights(true);
    outline_->setTextElideMode(Qt::ElideRight);
    outline_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    outline_->setFrameShape(QFrame::NoFrame);
    outline_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    const auto activate = [this](QTreeWidgetItem* item) {
        if (navigate && item) navigate(item->data(0, Qt::UserRole).toString());
    };
    connect(outline_, &QTreeWidget::itemClicked, this, activate);
    connect(outline_, &QTreeWidget::itemActivated, this, activate);
    layout->addWidget(outline_, 1);
}

void DocumentSidebar::setDocument(const QString& filename, const QJsonArray& outline) {
    filename_->setText(filename);
    reload_->setEnabled(true);
    outline_->clear();
    std::vector<std::pair<int, QTreeWidgetItem*>> parents;
    for (const auto& value : outline) {
        const auto entry = value.toObject();
        const int level = entry.value("level").toInt();
        while (!parents.empty() && parents.back().first >= level) parents.pop_back();
        auto* item = parents.empty() ? new QTreeWidgetItem(outline_) :
                                      new QTreeWidgetItem(parents.back().second);
        item->setText(0, entry.value("title").toString());
        item->setToolTip(0, item->text(0));
        item->setData(0, Qt::UserRole, entry.value("anchor").toString());
        parents.emplace_back(level, item);
    }
    outline_->expandAll();
}
} // namespace moonmark::qt
