/**
 * @file Theme.h
 * @brief Application-wide look and feel (Fusion style + palette + style sheet).
 */
#ifndef SC_UI_THEME_H
#define SC_UI_THEME_H

class QApplication;

namespace ui {

/**
 * @brief Applies the SecureCloud theme to the whole application.
 *
 * Call it once in main(), right after creating the QApplication and
 * before creating any window. Widgets are styled by their objectName
 * (see the style sheet in Theme.cpp), so a window never sets colors itself.
 *
 * @param app The running application.
 */
void applyTheme(QApplication& app);

} // namespace ui

#endif // SC_UI_THEME_H
