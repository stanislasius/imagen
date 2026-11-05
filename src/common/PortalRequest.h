// SPDX-License-Identifier: MIT
#pragma once

#include <QDBusConnection>
#include <QObject>
#include <QQueue>
#include <QString>
#include <QVariantMap>
#include <functional>

namespace imagen {

/**
 * Helper for the XDG desktop portal request/response dance.
 *
 * A portal method answers with a request object path; the actual result is
 * delivered later through the `Response` signal of the
 * `org.freedesktop.portal.Request` interface. The path of that object can
 * be predicted from the `handle_token` option, which lets the client
 * subscribe before the call is issued and therefore never miss a response.
 */
class PortalRequest : public QObject
{
    Q_OBJECT

public:
    /// Callback invoked with the response code and the result dictionary.
    using Callback = std::function<void(quint32 code, const QVariantMap &results)>;

    explicit PortalRequest(QObject *parent = nullptr);
    ~PortalRequest() override;

    /// The dedicated bus connection used for all portal traffic.
    QDBusConnection connection() const;

    /**
     * Calls a portal method.
     *
     * @param interface  portal interface, e.g. "org.freedesktop.portal.Screenshot"
     * @param method     method name
     * @param arguments  arguments placed before the options dictionary
     * @param options    options dictionary; `handle_token` is added automatically
     * @param callback   receives the response
     */
    void call(const QString &interface,
              const QString &method,
              const QList<QVariant> &arguments,
              QVariantMap options,
              const Callback &callback);

    /// The path the request object of the given token is expected at.
    QString requestPathForToken(const QString &token) const;

private Q_SLOTS:
    void onResponse(quint32 code, const QVariantMap &results);

private:
    QQueue<Callback> m_callbacks;
    mutable QDBusConnection m_connection;
    mutable bool m_dedicatedConnection = false;
};

} // namespace imagen
