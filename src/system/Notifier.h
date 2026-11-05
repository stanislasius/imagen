// SPDX-License-Identifier: MIT
#pragma once

#include <QObject>
#include <QString>

namespace imagen {

/**
 * Desktop notifications through the freedesktop notification service.
 */
class Notifier : public QObject
{
    Q_OBJECT

public:
    explicit Notifier(QObject *parent = nullptr);

    /// Whether a notification service is reachable.
    bool isAvailable() const;

    /// Shows a notification. Returns the id of the notification.
    uint notify(const QString &summary, const QString &body, const QString &iconName = QString());

    /// Shows a notification about a capture result.
    void notifyCapture(const QString &summary, const QString &body);

private:
    mutable bool m_available = false;
    mutable bool m_checked = false;
};

} // namespace imagen
