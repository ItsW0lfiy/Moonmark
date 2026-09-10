#pragma once

#include <QJsonArray>
#include <QStringList>
#include <QWidget>
#include <functional>

class QLabel;
class QPushButton;
class QTreeWidget;

namespace moonmark::qt {
class DocumentSidebar final : public QWidget {
public:
    explicit DocumentSidebar(QWidget* parent = nullptr);
    void setDocuments(const QStringList& filenames, int active_index);
    void setOutline(const QJsonArray& outline);
    std::function<void()> open;
    std::function<void()> reload;
    std::function<void(const QString&)> navigate;
    std::function<void(int)> activate_document;
    std::function<void(int)> close_document;

private:
    QPushButton* reload_;
    QTreeWidget* documents_;
    QTreeWidget* outline_;
};
} // namespace moonmark::qt
