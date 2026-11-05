// SPDX-License-Identifier: MIT
#pragma once

#include "capture/CaptureTypes.h"

#include <QObject>
#include <QRect>

namespace imagen {

/**
 * Remembers what was captured last.
 *
 * The information is used by the "repeat last capture" action and by the
 * "fixed region" mode, which reuses the rectangle of the previous region
 * capture. It is persisted in the settings file so the behaviour survives
 * a restart.
 */
class CaptureHistory : public QObject
{
    Q_OBJECT

public:
    explicit CaptureHistory(QObject *parent = nullptr);

    /// The rectangle of the last capture in logical desktop coordinates.
    QRect lastRegion() const { return m_lastRegion; }

    /// The kind of the last capture.
    CaptureKind lastKind() const { return m_lastKind; }

    /// Whether a usable region was remembered.
    bool hasRegion() const { return m_lastRegion.isValid() && !m_lastRegion.isEmpty(); }

    /// Stores the request that produced the current capture.
    void remember(const CaptureRequest &request);

    /// Stores a rectangle explicitly.
    void rememberRegion(const QRect &region, CaptureKind kind = CaptureKind::Region);

    /// Forgets the remembered region.
    void clear();

Q_SIGNALS:
    void changed();

private:
    void load();
    void save() const;

    QRect m_lastRegion;
    CaptureKind m_lastKind = CaptureKind::FullScreen;
};

} // namespace imagen
