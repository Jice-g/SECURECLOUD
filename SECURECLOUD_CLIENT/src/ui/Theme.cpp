#include "ui/Theme.h"

#include <QApplication>
#include <QColor>
#include <QFont>
#include <QPalette>
#include <QString>
#include <QStyleFactory>

namespace ui {

namespace {

// Colour tokens, kept in one place. The style sheet below repeats the same
// values (QSS has no variables): change both together.
//   navy      #131B33   header / brand panel
//   primary   #3B5BDB   buttons, focus ring, selection
//   surface   #F4F6FA   window background
//   border    #D9DEE8   separators and frames
//   text      #1B2333   main text
//   muted     #6B7488   secondary text
const char* const STYLE_SHEET = R"qss(

* ---------- Windows ---------- */

QDialog#UserLoginWindow,
QMainWindow#ClientChatWindow {
    background: #F4F6FA;
}

/* ---------- Login: brand panel ---------- */
QFrame#brandPanel {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #131B33, stop:1 #2A3F7A);
}
QLabel#logoLabel {
    background: rgba(255, 255, 255, 30);
    border: 1px solid rgba(255, 255, 255, 70);
    border-radius: 16px;
    color: #FFFFFF;
    font-size: 20pt;
    font-weight: 700;
}
QLabel#titleLabel   { color: #FFFFFF; font-size: 18pt; font-weight: 600; }
QLabel#taglineLabel { color: #B4C0E4; }

/* ---------- Login: form ---------- */
QWidget#formPanel { background: #FFFFFF; }
QLabel#headingLabel  { color: #1B2333; font-size: 15pt; font-weight: 600; }
QLabel#loginLabel,
QLabel#passwordLabel { color: #4A5468; font-weight: 600; }
QLabel#footerLabel   { color: #8A93A6; font-size: 8pt; }
QCheckBox            { color: #4A5468; spacing: 8px; }

QLineEdit, QPlainTextEdit {
    background: #FFFFFF;
    border: 1px solid #CBD2E0;
    border-radius: 8px;
    padding: 8px 12px;
    selection-background-color: #3B5BDB;
    selection-color: #FFFFFF;
}
QLineEdit:focus, QPlainTextEdit:focus { border: 1px solid #3B5BDB; }
QLineEdit:disabled, QPlainTextEdit:disabled {
    background: #EEF0F5;
    color: #8A93A6;
    border: 1px solid #E1E5EE;
}

/* Message banner: the "state" property is set from C++ (error / warning). */
QLabel#messageLabel { border-radius: 8px; padding: 9px 12px; }
QLabel#messageLabel[state="error"] {
    background: #FFF0F0; color: #A61E1E; border: 1px solid #FFC9C9;
}
QLabel#messageLabel[state="warning"] {
    background: #FFF8E6; color: #8A5A00; border: 1px solid #FFE2A1;
}

/* ---------- Primary buttons ---------- */
QPushButton#signInButton,
QPushButton#sendButton {
    background: #3B5BDB;
    color: #FFFFFF;
    border: none;
    border-radius: 8px;
    padding: 10px 20px;
    font-weight: 600;
}
QPushButton#signInButton:hover,
QPushButton#sendButton:hover   { background: #364FC7; }
QPushButton#signInButton:pressed,
QPushButton#sendButton:pressed { background: #2F44AD; }
QPushButton#signInButton:disabled,
QPushButton#sendButton:disabled { background: #BAC4EA; color: #F4F6FF; }

/* Secondary button */
QPushButton#findContactButton {
    background: #FFFFFF;
    color: #3B5BDB;
    border: 1px solid #CBD2E0;
    border-radius: 8px;
    padding: 8px 14px;
    font-weight: 600;
}
QPushButton#findContactButton:hover    { background: #F0F3FF; }
QPushButton#findContactButton:disabled { color: #A3ABBD; background: #F4F6FA; }

/* ---------- Main window: header bar ---------- */
QFrame#headerBar { background: #131B33; border: none; }
QLabel#appTitleLabel { color: #FFFFFF; font-size: 14pt; font-weight: 600; }
QLabel#userNameLabel { color: #E4E8F5; font-weight: 600; }
QLabel#roleBadgeLabel {
    background: #3B5BDB;
    color: #FFFFFF;
    border-radius: 9px;
    padding: 2px 10px;
    font-size: 8pt;
    font-weight: 700;
}
QPushButton#signOutButton {
    background: transparent;
    color: #E4E8F5;
    border: 1px solid rgba(255, 255, 255, 90);
    border-radius: 8px;
    padding: 6px 14px;
}
QPushButton#signOutButton:hover   { background: rgba(255, 255, 255, 28); }
QPushButton#signOutButton:pressed { background: rgba(255, 255, 255, 50); }

/* ---------- Main window: panels ---------- */
QFrame#sidePanel {
    background: #FFFFFF;
    border: none;
    border-right: 1px solid #D9DEE8;
}
QFrame#conversationPanel { background: #F4F6FA; border: none; }

QLabel#contactsTitleLabel,
QLabel#activityTitleLabel { color: #6B7488; font-size: 8pt; font-weight: 700; }
QLabel#conversationTitleLabel { color: #1B2333; font-size: 12pt; font-weight: 600; }

QListWidget#contactList {
    background: #FFFFFF;
    border: 1px solid #E3E7F0;
    border-radius: 8px;
    outline: 0;
}
QListWidget#contactList::item           { padding: 9px 10px; }
QListWidget#contactList::item:selected  { background: #E7ECFF; color: #1B2333; }

QTextBrowser#conversationView {
    background: #FFFFFF;
    border: 1px solid #E3E7F0;
    border-radius: 10px;
    padding: 8px;
}
QTextBrowser#activityLogView {
    background: #0F1526;
    color: #B7F26B;
    border: none;
    border-radius: 8px;
    padding: 8px;
    font-family: "Cascadia Mono", Consolas, Menlo, "DejaVu Sans Mono", monospace;
    font-size: 8pt;
}

QSplitter::handle { background: #D9DEE8; }
QSplitter::handle:horizontal { width: 1px; }

QStatusBar {
    background: #FFFFFF;
    color: #6B7488;
    border-top: 1px solid #D9DEE8;
}

)qss";

} // namespace

void applyTheme(QApplication& app)
{
    // Fusion renders identically on Windows, Linux and macOS, which makes
    // the style sheet predictable. (The native Windows style ignores parts of QSS.)
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));

    QPalette palette;
    const QColor surface(0xF4, 0xF6, 0xFA);
    const QColor text(0x1B, 0x23, 0x33);
    const QColor primary(0x3B, 0x5B, 0xDB);
    const QColor disabledText(0x8A, 0x93, 0xA6);

    palette.setColor(QPalette::Window, surface);
    palette.setColor(QPalette::WindowText, text);
    palette.setColor(QPalette::Base, Qt::white);
    palette.setColor(QPalette::AlternateBase, surface);
    palette.setColor(QPalette::Text, text);
    palette.setColor(QPalette::Button, Qt::white);
    palette.setColor(QPalette::ButtonText, text);
    palette.setColor(QPalette::Highlight, primary);
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::PlaceholderText, QColor(0x9A, 0xA3, 0xB5));
    palette.setColor(QPalette::Disabled, QPalette::Text, disabledText);
    palette.setColor(QPalette::Disabled, QPalette::WindowText, disabledText);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledText);
    app.setPalette(palette);

    // No font family is forced: each OS keeps its own UI font (Segoe UI, SF, ...).
    QFont font = app.font();
    font.setPointSizeF(10.0);
    app.setFont(font);

    app.setStyleSheet(QString::fromUtf8(STYLE_SHEET));
}

} // namespace ui
