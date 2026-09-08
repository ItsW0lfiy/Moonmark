#pragma once

#include <QWidget>
#include <QAbstractButton>

namespace moonmark::qt {

class CaptionButton final : public QAbstractButton {
public:
    enum class Action { Minimize, Maximize, Close };
    explicit CaptionButton(Action action, QWidget* parent = nullptr);
    void setMaximized(bool maximized);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Action action_;
    bool maximized_ = false;
};

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
