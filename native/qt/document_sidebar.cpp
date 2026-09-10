#include "document_sidebar.h"
#include "moon_style.h"

#include <QApplication>
#include <QBoxLayout>
#include <QHeaderView>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <algorithm>
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
    open_button->setToolTip(QStringLiteral("Open Markdown or text file (Ctrl+O)"));
    connect(open_button, &QPushButton::clicked, this, [this] { if (open) open(); });
    layout->addWidget(open_button);
    reload_ = new QPushButton(QStringLiteral("Reload"));
    reload_->setMinimumHeight(36);
    reload_->setEnabled(false);
    reload_->setToolTip(QStringLiteral("Reload current document (F5)"));
    connect(reload_, &QPushButton::clicked, this, [this] { if (reload) reload(); });
    layout->addWidget(reload_);
    layout->addSpacing(28);
    auto* label = new QLabel(QStringLiteral("OPEN DOCUMENTS"));
    label->setObjectName(QStringLiteral("sidebarSection"));
    layout->addWidget(label);
    documents_ = new QTreeWidget;
    documents_->setObjectName(QStringLiteral("openDocuments"));
    documents_->setAccessibleName(QStringLiteral("Open documents"));
    documents_->setHeaderHidden(true);
    documents_->setColumnCount(2);
    documents_->setRootIsDecorated(false);
    documents_->setIndentation(0);
    documents_->setUniformRowHeights(true);
    documents_->setTextElideMode(Qt::ElideMiddle);
    documents_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    documents_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    documents_->setFrameShape(QFrame::NoFrame);
    documents_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    documents_->header()->setStretchLastSection(false);
    documents_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    documents_->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    documents_->setColumnWidth(1, 28);
    connect(documents_, &QTreeWidget::itemClicked, this,
            [this](QTreeWidgetItem* item, int column) {
        if (!item) return;
        const int index = item->data(0, Qt::UserRole).toInt();
        if (column == 1) {
            if (close_document) close_document(index);
        } else if (activate_document) {
            activate_document(index);
        }
    });
    connect(documents_, &QTreeWidget::itemActivated, this,
            [this](QTreeWidgetItem* item, int) {
        if (activate_document && item)
            activate_document(item->data(0, Qt::UserRole).toInt());
    });
    layout->addWidget(documents_);
    layout->addSpacing(20);
    auto* outline_label = new QLabel(QStringLiteral("OUTLINE"));
    outline_label->setObjectName(QStringLiteral("sidebarSection"));
    layout->addWidget(outline_label);
    outline_ = new QTreeWidget;
    outline_->setObjectName(QStringLiteral("documentOutline"));
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

void DocumentSidebar::setDocuments(const QStringList& filenames, int active_index) {
    documents_->clear();
    for (int index = 0; index < filenames.size(); ++index) {
        auto* item = new QTreeWidgetItem(documents_);
        item->setText(0, filenames.at(index));
        item->setText(1, QStringLiteral("×"));
        item->setTextAlignment(1, Qt::AlignCenter);
        item->setToolTip(0, filenames.at(index));
        item->setToolTip(1, QStringLiteral("Close document"));
        item->setData(0, Qt::UserRole, index);
        if (index == active_index) documents_->setCurrentItem(item);
    }
    const int rows = std::clamp(static_cast<int>(filenames.size()), 1, 4);
    documents_->setFixedHeight(rows * 34 + 4);
    reload_->setEnabled(active_index >= 0);
}

void DocumentSidebar::setOutline(const QJsonArray& outline) {
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
