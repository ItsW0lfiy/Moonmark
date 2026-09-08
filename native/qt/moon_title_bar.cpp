#include "moon_title_bar.h"

#include "moon_style.h"

#include <QMouseEvent>
#include <QWindow>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace moonmark::qt {

MoonTitleBar::MoonTitleBar(QWidget* window) : QWidget(window), window_(window) {
    setFixedHeight(style::metric::title_bar_height);
    setObjectName(QStringLiteral("titleBar"));
}

void MoonTitleBar::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && window_->windowHandle() != nullptr) {
        window_->windowHandle()->startSystemMove();
        event->accept();
        return;
    }
#ifdef _WIN32
    if (event->button() == Qt::RightButton) {
        const auto global = event->globalPosition().toPoint();
        const auto handle = reinterpret_cast<HWND>(window_->winId());
        const auto menu = GetSystemMenu(handle, FALSE);
        const auto command = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON, global.x(),
                                            global.y(), 0, handle, nullptr);
        if (command != 0) {
            PostMessageW(handle, WM_SYSCOMMAND, command, 0);
        }
        event->accept();
        return;
    }
#endif
    QWidget::mousePressEvent(event);
}

void MoonTitleBar::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        window_->isMaximized() ? window_->showNormal() : window_->showMaximized();
        event->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

} // namespace moonmark::qt
