// SPDX-License-Identifier: MIT
#pragma once

#include "capture/CaptureTypes.h"

#include <QObject>
#include <QString>

namespace imagen {

/**
 * Abstract interface every capture backend implements.
 *
 * A backend is a one-shot, asynchronous producer of images. It is fed a
 * CaptureRequest and answers with exactly one of: frameReady(),
 * captureCancelled() or captureFailed().
 */
class CaptureBackend : public QObject
{
    Q_OBJECT

public:
    explicit CaptureBackend(QObject *parent = nullptr);
    ~CaptureBackend() override = default;

    /// Stable identifier, e.g. "portal" or "x11".
    virtual QString id() const = 0;

    /// Human readable name shown in the diagnostics dialog.
    virtual QString displayName() const = 0;

    /// Whether the backend can be used in the current session.
    virtual bool isAvailable() const = 0;

    /// Whether the backend implements the given capture kind.
    virtual bool supports(CaptureKind kind) const = 0;

    /// Starts a capture. The result arrives through the signals below.
    virtual void requestCapture(const CaptureRequest &request) = 0;

    /// Aborts a running capture, if the backend supports it.
    virtual void cancel() {}

Q_SIGNALS:
    void frameReady(const imagen::CapturedFrame &frame);
    void captureCancelled();
    void captureFailed(const QString &message);
};

} // namespace imagen
