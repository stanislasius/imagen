// SPDX-License-Identifier: MIT
#pragma once

#include <QObject>
#include <QStringList>

class QAction;
class QMenu;
class QSystemTrayIcon;

namespace imagen {

/**
 * Tray icon with the main menu.
 *
 * The menu is built from a list of action identifiers so the application
 * can handle the entries without knowing about the menu structure.
 */
class TrayIcon : public QObject
{
    Q_OBJECT

public:
    explicit TrayIcon(QObject *parent = nullptr);
    ~TrayIcon() override;

    /// Shows or hides the tray icon.
    void setVisible(bool visible);
    bool isVisible() const;

    /// Whether the system tray is available at all.
    static bool isSystemTrayAvailable();

    /// The action identifiers the menu offers, in order.
    QStringList actions() const;

    /// Sets the tool tip of the tray icon.
    void setToolTip(const QString &text);

Q_SIGNALS:
    /// Emitted with the identifier of the chosen menu entry.
    void actionTriggered(const QString &actionId);

private:
    void buildMenu();

    QSystemTrayIcon *m_tray = nullptr;
    QMenu *m_menu = nullptr;
    QAction *m_captureMenuAction = nullptr;
};

} // namespace imagen
