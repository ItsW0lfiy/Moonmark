#pragma once

#include <QJsonArray>
#include <QWidget>
#include <functional>

class QLabel;
class QPushButton;
class QTreeWidget;

namespace moonmark::qt {
class DocumentSidebar final : public QWidget {
public:
    explicit DocumentSidebar(QWidget* parent = nullptr);
    void setDocument(const QString& filename, const QJsonArray& outline);
    std::function<void()> open;
    std::function<void()> reload;
    std::function<void(const QString&)> navigate;

private:
    QLabel* filename_;
    QPushButton* reload_;
    QTreeWidget* outline_;
};
} // namespace moonmark::qt
