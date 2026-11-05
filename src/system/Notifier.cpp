// SPDX-License-Identifier: MIT
#include "system/Notifier.h"

#include "common/AppInfo.h"
#include "common/Logging.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>
#include <QVariantMap>

namespace imagen {
namespace {

const char *kService = "org.freedesktop.Notifications";
const char *kPath = "/org/freedesktop/Notifications";
const char *kInterface = "org.freedesktop.Notifications";

} // namespace

Notifier::Notifier(QObject *parent)
    : QObject(parent)
{
}

bool Notifier::isAvailable() const
{
    if (m_checked)
        return m_available;

    m_checked = true;
    const QDBusConnection bus = QDBusConnection::sessionBus();
    m_available = bus.isConnected()
            && bus.interface()
            && bus.interface()->isServiceRegistered(QLatin1String(kService)).value();
    return m_available;
}

uint Notifier::notify(const QString &summary, const QString &body, const QString &iconName)
{
    if (!isAvailable()) {
        qCDebug(lcApp) << "no notification service, skipping notification";
        return 0;
    }

    QDBusInterface interface(QLatin1String(kService), QLatin1String(kPath), QLatin1String(kInterface),
                             QDBusConnection::sessionBus());
    if (!interface.isValid()) {
        qCDebug(lcApp) << "the notification interface is invalid";
        return 0;
    }

    QDBusReply<uint> reply = interface.call(QStringLiteral("Notify"),
                                             QLatin1String(AppInfo::ApplicationName),
                                             0u,
                                             iconName.isEmpty() ? QLatin1String(AppInfo::AppId) : iconName,
                                             summary,
                                             body,
                                             QStringList(),
                                             QVariantMap(),
                                             5000);
    if (!reply.isValid())
        qCDebug(lcApp) << "the notification was rejected:" << reply.error().message();

    return reply.isValid() ? reply.value() : 0;
}

void Notifier::notifyCapture(const QString &summary, const QString &body)
{
    notify(summary, body, QStringLiteral("camera-photo"));
}

} // namespace imagen
