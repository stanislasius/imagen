// SPDX-License-Identifier: MIT
#include "tray/TrayIcon.h"

#include "common/AppInfo.h"
#include "common/Logging.h"
#include "settings/Config.h"

#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QMenu>
#include <QSystemTrayIcon>

namespace imagen {

TrayIcon::TrayIcon(QObject *parent)
    : QObject(parent)
    , m_tray(new QSystemTrayIcon(this))
    , m_menu(new QMenu())
{
    m_tray->setContextMenu(m_menu);

    QIcon icon = QIcon::fromTheme(QStringLiteral("camera-photo"));
    if (icon.isNull())
        icon = QIcon(QStringLiteral(":/icons/org.imagen.Imagen.svg"));
    m_tray->setIcon(icon);
    m_tray->setToolTip(QStringLiteral("imagen"));

    connect(m_tray, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger)
            Q_EMIT actionTriggered(QStringLiteral("shell.traymenu"));
    });

    buildMenu();
}

TrayIcon::~TrayIcon()
{
    m_tray->hide();
}

bool TrayIcon::isSystemTrayAvailable()
{
    return QSystemTrayIcon::isSystemTrayAvailable();
}

void TrayIcon::setVisible(bool visible)
{
    if (visible) {
        m_tray->show();
    } else {
        m_tray->hide();
    }
}

bool TrayIcon::isVisible() const
{
    return m_tray->isVisible();
}

QStringList TrayIcon::actions() const
{
    return { QStringLiteral("capture.fullscreen"),
             QStringLiteral("capture.monitor"),
             QStringLiteral("capture.activewindow"),
             QStringLiteral("capture.window"),
             QStringLiteral("capture.region"),
             QStringLiteral("capture.fixedregion"),
             QStringLiteral("capture.last"),
             QStringLiteral("editor.open"),
             QStringLiteral("shell.settings"),
             QStringLiteral("app.quit") };
}

void TrayIcon::setToolTip(const QString &text)
{
    m_tray->setToolTip(text);
}

void TrayIcon::buildMenu()
{
    m_menu->clear();

    QAction *fullScreen = m_menu->addAction(tr("Capture full screen"));
    fullScreen->setData(QStringLiteral("capture.fullscreen"));
    connect(fullScreen, &QAction::triggered, this, [this]() {
        Q_EMIT actionTriggered(QStringLiteral("capture.fullscreen"));
    });

    QMenu *captureMenu = m_menu->addMenu(tr("Capture"));
    m_captureMenuAction = captureMenu->menuAction();

    struct Entry {
        const char *id;
        const char *label;
    };

    const Entry entries[] = {
        { "capture.fullscreen", QT_TR_NOOP("Full screen") },
        { "capture.monitor", QT_TR_NOOP("Single monitor") },
        { "capture.activewindow", QT_TR_NOOP("Active window") },
        { "capture.window", QT_TR_NOOP("Window") },
        { "capture.region", QT_TR_NOOP("Region") },
        { "capture.fixedregion", QT_TR_NOOP("Fixed region") },
        { "capture.freehand", QT_TR_NOOP("Freehand region") },
        { "capture.last", QT_TR_NOOP("Repeat last capture") },
    };

    for (const Entry &entry : entries) {
        QAction *action = captureMenu->addAction(tr(entry.label));
        const QString id = QString::fromLatin1(entry.id);
        connect(action, &QAction::triggered, this, [this, id]() { Q_EMIT actionTriggered(id); });
    }

    m_menu->addSeparator();

    QAction *editor = m_menu->addAction(tr("Open the editor"));
    connect(editor, &QAction::triggered, this, [this]() {
        Q_EMIT actionTriggered(QStringLiteral("editor.open"));
    });

    QAction *settings = m_menu->addAction(tr("Settings"));
    connect(settings, &QAction::triggered, this, [this]() {
        Q_EMIT actionTriggered(QStringLiteral("shell.settings"));
    });

    m_menu->addSeparator();

    QAction *quit = m_menu->addAction(tr("Quit"));
    connect(quit, &QAction::triggered, this, [this]() {
        Q_EMIT actionTriggered(QStringLiteral("app.quit"));
    });
}

} // namespace imagen
