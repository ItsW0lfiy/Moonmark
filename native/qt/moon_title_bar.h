#pragma once

#include <QWidget>

namespace moonmark::qt {

class MoonTitleBar final : public QWidget {
public:
    explicit MoonTitleBar(QWidget* window);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    QWidget* window_ = nullptr;
};

} // namespace moonmark::qt
