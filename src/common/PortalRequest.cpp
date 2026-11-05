// SPDX-License-Identifier: MIT
#include "common/PortalRequest.h"

#include "common/AppInfo.h"
#include "common/Logging.h"

#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QUuid>

namespace imagen {
namespace {

const char *kPortalService = "org.freedesktop.portal.Desktop";
const char *kPortalPath = "/org/freedesktop/portal/desktop";
const char *kRequestInterface = "org.freedesktop.portal.Request";

} // namespace

PortalRequest::PortalRequest(QObject *parent)
    : QObject(parent)
    , m_connection(QDBusConnection::sessionBus())
{
}

PortalRequest::~PortalRequest() = default;

QDBusConnection PortalRequest::connection() const
{
    // The handle is kept for the lifetime of the object: dropping it
    // would close the underlying bus connection while calls are in flight.
    if (!m_dedicatedConnection) {
        m_dedicatedConnection = true;
        m_connection = QDBusConnection::connectToBus(QDBusConnection::SessionBus,
                                                     QStringLiteral("imagen-portal"));
    }
    return m_connection;
}

QString PortalRequest::requestPathForToken(const QString &token) const
{
    // The connection of this object is used on purpose: creating another
    // handle for the same bus connection can invalidate the existing one.
    QDBusConnection bus = connection();
    QString sender = bus.baseService();
    if (sender.startsWith(QLatin1Char(':')))
        sender.remove(0, 1);
    sender.replace(QLatin1Char('.'), QLatin1Char('_'));
    return QStringLiteral("%1/request/%2/%3").arg(QLatin1String(kPortalPath), sender, token);
}

void PortalRequest::call(const QString &interface,
                         const QString &method,
                         const QList<QVariant> &arguments,
                         QVariantMap options,
                         const Callback &callback)
{
    QDBusConnection bus = connection();
    if (!bus.isConnected()) {
        qCWarning(lcPlatform) << "no session bus for the portal request";
        callback(2, {});
        return;
    }

    // The handle token becomes the last element of the request object path.
    // Object path elements only allow [A-Za-z0-9_], so neither dots nor
    // dashes may appear here.
    QString token = QUuid::createUuid().toString(QUuid::Id128);
    token.remove(QLatin1Char('-'));
    token = QStringLiteral("imagen_%1").arg(token);
    options.insert(QStringLiteral("handle_token"), token);

    // Subscribe before the call: the portal may answer immediately.
    const QString predictedPath = requestPathForToken(token);
    if (!bus.connect(QLatin1String(kPortalService),
                     predictedPath,
                     QLatin1String(kRequestInterface),
                     QStringLiteral("Response"),
                     this,
                     SLOT(onResponse(quint32, QVariantMap)))) {
        qCWarning(lcPlatform) << "cannot subscribe to the portal response";
    }

    QDBusMessage message = QDBusMessage::createMethodCall(QLatin1String(kPortalService),
                                                          QLatin1String(kPortalPath),
                                                          interface,
                                                          method);
    QList<QVariant> allArguments = arguments;
    allArguments.append(QVariant::fromValue(options));
    message.setArguments(allArguments);

    m_callbacks.enqueue(callback);

    qCDebug(lcPlatform) << "portal call" << method << "on connection" << bus.name()
                         << "connected:" << bus.isConnected();

    auto *watcher = new QDBusPendingCallWatcher(bus.asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, predictedPath, bus, method]() {
        watcher->deleteLater();
        const QDBusMessage reply = watcher->reply();
        qCDebug(lcPlatform) << "portal reply for" << method << "type" << reply.type()
                             << "connected:" << bus.isConnected();
        if (reply.type() == QDBusMessage::ErrorMessage) {
            qCWarning(lcPlatform) << "portal call failed:" << reply.errorMessage();
            if (!m_callbacks.isEmpty())
                m_callbacks.dequeue()(2, {});
            return;
        }
        if (reply.arguments().isEmpty())
            return;

        // The portal may use a different path than predicted; subscribe as well.
        const QString actualPath = reply.arguments().first().toString();
        if (actualPath.isEmpty() || actualPath == predictedPath)
            return;

        QDBusConnection fallback = connection();
        fallback.connect(QLatin1String(kPortalService),
                         actualPath,
                         QLatin1String(kRequestInterface),
                         QStringLiteral("Response"),
                         this,
                         SLOT(onResponse(quint32, QVariantMap)));
    });
}

void PortalRequest::onResponse(quint32 code, const QVariantMap &results)
{
    if (m_callbacks.isEmpty()) {
        qCDebug(lcPlatform) << "unexpected portal response, code" << code;
        return;
    }
    m_callbacks.dequeue()(code, results);
}

} // namespace imagen
