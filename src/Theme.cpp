#include "Theme.h"

#include <QApplication>
#include <QString>

namespace {

// Palette matches the app icon and documentation: navy background,
// a slightly lighter navy for panels/inputs, and the same gold used
// as the icon's accent dot for focus/selection highlights.
const QString kDarkStylesheet = R"(
QWidget {
    background-color: #1a1a2e;
    color: #e6e6ef;
}

QMainWindow, QDialog {
    background-color: #1a1a2e;
}

QLabel {
    color: #e6e6ef;
    background: transparent;
}

QLineEdit, QTextEdit, QComboBox {
    background-color: #22223a;
    color: #e6e6ef;
    border: 1px solid #3a3a5c;
    border-radius: 4px;
    padding: 4px;
    selection-background-color: #e9c46a;
    selection-color: #1a1a2e;
}

QLineEdit:focus, QTextEdit:focus, QComboBox:focus {
    border: 1px solid #e9c46a;
}

QPushButton {
    background-color: #0f3460;
    color: #f4f4f7;
    border: 1px solid #16467d;
    border-radius: 4px;
    padding: 6px 14px;
}

QPushButton:hover {
    background-color: #16467d;
}

QPushButton:pressed {
    background-color: #0a2647;
}

QPushButton:disabled {
    background-color: #2a2a45;
    color: #6b6b85;
    border: 1px solid #2a2a45;
}

QPushButton:checked {
    background-color: #e9c46a;
    color: #1a1a2e;
    border: 1px solid #e9c46a;
}

QCheckBox, QRadioButton {
    color: #e6e6ef;
    background: transparent;
}

QCheckBox::indicator, QRadioButton::indicator {
    width: 14px;
    height: 14px;
    border: 1px solid #5a5a80;
    border-radius: 3px;
    background-color: #22223a;
}

QCheckBox::indicator:hover, QRadioButton::indicator:hover {
    border: 1px solid #e9c46a;
}

QCheckBox::indicator:checked, QRadioButton::indicator:checked {
    background-color: #e9c46a;
    border: 1px solid #e9c46a;
}

QTableWidget {
    background-color: #22223a;
    alternate-background-color: #262643;
    gridline-color: #3a3a5c;
    color: #e6e6ef;
    border: 1px solid #3a3a5c;
    selection-background-color: #0f3460;
    selection-color: #ffffff;
}

QHeaderView::section {
    background-color: #0f3460;
    color: #f4f4f7;
    padding: 6px;
    border: 1px solid #16467d;
}

QMenuBar {
    background-color: #16213e;
    color: #e6e6ef;
}

QMenuBar::item:selected {
    background-color: #0f3460;
}

QMenu {
    background-color: #22223a;
    color: #e6e6ef;
    border: 1px solid #3a3a5c;
}

QMenu::item:selected {
    background-color: #0f3460;
}

QScrollBar:vertical, QScrollBar:horizontal {
    background: #1a1a2e;
    border: none;
}

QScrollBar::handle {
    background: #3a3a5c;
    border-radius: 4px;
}

QScrollBar::handle:hover {
    background: #4a4a72;
}

QMessageBox {
    background-color: #1a1a2e;
}
)";

} // namespace

namespace Theme {

void apply(bool darkModeEnabled)
{
    if (QApplication *app = qApp) {
        app->setStyleSheet(darkModeEnabled ? kDarkStylesheet : QString());
    }
}

} // namespace Theme
