#include "moon_style.h"

#include <QApplication>
#include <QColor>
#include <QPalette>

namespace moonmark::style {

void apply(QApplication& application) {
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(colour::background));
    palette.setColor(QPalette::WindowText, QColor(colour::text));
    palette.setColor(QPalette::Base, QColor(colour::document));
    palette.setColor(QPalette::AlternateBase, QColor(colour::surface));
    palette.setColor(QPalette::Text, QColor(colour::text));
    palette.setColor(QPalette::Button, QColor(colour::surface));
    palette.setColor(QPalette::ButtonText, QColor(colour::text));
    palette.setColor(QPalette::Highlight, QColor(colour::selection));
    palette.setColor(QPalette::HighlightedText, QColor(colour::bright));
    palette.setColor(QPalette::Link, QColor(colour::silver));
    palette.setColor(QPalette::LinkVisited, QColor(colour::secondary));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(colour::muted));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(colour::muted));
    application.setPalette(palette);
    application.setStyleSheet(QStringLiteral(R"(
        QWidget { background: #080808; color: #e8e8e8; font-family: "Segoe UI Variable Text", "Segoe UI"; }
        #titleBar { background: #0c0c0c; border-bottom: 1px solid #252525; }
        #commandBar { background: #101010; border-bottom: 1px solid #282828; }
        QPushButton { background: #171717; border: 1px solid #303030; border-radius: 5px; padding: 4px 11px; }
        QPushButton:hover { background: #242424; border-color: #464646; }
        QPushButton:pressed { background: #303030; }
        QPushButton:focus { border: 1px solid #c8c8c8; }
        QPushButton:disabled { color: #525252; background: #101010; border-color: #242424; }
        #primaryAction { background: #dedede; color: #111111; border-color: #dedede; font-weight: 600; }
        #primaryAction:hover { background: #f0f0f0; border-color: #f0f0f0; }
        #primaryAction:pressed { background: #c2c2c2; border-color: #c2c2c2; }
        #primaryAction:focus { border: 1px solid #ffffff; }
        #toolbarButton { background: transparent; border-color: #383838; }
        #toolbarButton:hover { background: #242424; border-color: #505050; }
        #toolbarButton:pressed { background: #303030; }
        #commandSeparator { color: #353535; }
        #captionButton { border: 0; border-radius: 0; background: transparent; font-size: 15px; padding: 0; }
        #captionButton:hover { background: #242424; }
        #captionButton:pressed { background: #303030; }
        #closeButton:hover { background: #612f2f; color: #ffffff; }
        #documentTitle { color: #d8d8d8; font-size: 12px; font-weight: 600; }
        #documentContext { color: #777777; font-size: 11px; }
        #zoomControl { background: #151515; border: 1px solid #303030; border-radius: 6px; }
        #zoomButton { border: 0; border-radius: 4px; background: transparent; padding: 0; }
        #zoomButton:hover { background: #282828; }
        #zoomButton:pressed { background: #363636; }
        #zoomValue { background: transparent; border: 0; color: #b8b8b8; font-size: 11px; }
        #documentStack { background: #0e0e0e; }
        #emptyTitle { font-size: 25px; font-weight: 600; color: #f0f0f0; }
        #emptyHint { color: #858585; margin-top: 4px; }
        #diagnosticsBar { background: #101010; border-top: 1px solid #282828; color: #777777; font-size: 11px; }
        QTextEdit { background: #0e0e0e; border: 0; selection-background-color: #484848; selection-color: #f0f0f0; }
        QScrollBar:vertical { background: #0e0e0e; width: 13px; margin: 0; }
        QScrollBar::handle:vertical { background: #464646; min-height: 30px; border-radius: 5px; margin: 2px; }
        QScrollBar::handle:vertical:hover { background: #5c5c5c; }
        QScrollBar:horizontal { background: #0e0e0e; height: 13px; margin: 0; }
        QScrollBar::handle:horizontal { background: #464646; min-width: 30px; border-radius: 5px; margin: 2px; }
        QScrollBar::handle:horizontal:hover { background: #5c5c5c; }
        QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }
        QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }
        QToolTip { background: #1c1c1c; color: #e8e8e8; border: 1px solid #464646; padding: 4px; }
        QMenu { background: #141414; color: #e8e8e8; border: 1px solid #303030; padding: 5px; }
        QMenu::item { border-radius: 4px; padding: 6px 24px 6px 10px; }
        QMenu::item:selected { background: #303030; color: #f0f0f0; }
        QMenu::separator { height: 1px; background: #303030; margin: 5px 8px; }
    )"));
}

} // namespace moonmark::style
