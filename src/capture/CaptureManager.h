// SPDX-License-Identifier: MIT
#pragma once

#include "capture/CaptureBackend.h"
#include "capture/CaptureTypes.h"

#include <QObject>

namespace imagen {

class CaptureHistory;
class DelayCountdown;

/**
 * Entry point of the capture subsystem.
 *
 * Chooses the backend that fits the current session, applies the
 * configured delay and forwards the resulting frame to the application.
 */
class CaptureManager : public QObject
{
    Q_OBJECT

public:
    explicit CaptureManager(QObject *parent = nullptr);
    ~CaptureManager() override;

    /// Whether at least one backend is usable.
    bool hasBackend() const;

    /// Name of the backend that would be used, empty when none is usable.
    QString backendName() const;

    /// All instantiated backends, usable for diagnostics.
    QList<CaptureBackend *> backends() const;

    /// Starts a capture of the given kind.
    void requestCapture(CaptureKind kind);

    /// Starts a capture with a fully specified request.
    void requestCapture(const CaptureRequest &request);

    /// Aborts a running capture.
    void cancel();

    /// The capture history, used for "repeat last capture".
    CaptureHistory *history() const { return m_history; }

Q_SIGNALS:
    void frameReady(const imagen::CapturedFrame &frame);
    void captureCancelled();
    void captureFailed(const QString &message);

private:
    void startBackendCapture(const CaptureRequest &request);
    /// The backends in the order they are tried for the current session.
    QList<CaptureBackend *> preferredBackends() const;
    void onBackendFrame(const imagen::CapturedFrame &frame);
    void onBackendCancelled();
    void onBackendFailed(const QString &message);

    QList<CaptureBackend *> m_backends;
    CaptureBackend *m_activeBackend = nullptr;
    CaptureHistory *m_history = nullptr;
    DelayCountdown *m_countdown = nullptr;
    CaptureRequest m_pendingRequest;
};

} // namespace imagen
