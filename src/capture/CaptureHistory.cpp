// SPDX-License-Identifier: MIT
#include "capture/CaptureHistory.h"

#include "common/Logging.h"
#include "settings/Config.h"

namespace imagen {

CaptureHistory::CaptureHistory(QObject *parent)
    : QObject(parent)
{
    load();
}

void CaptureHistory::remember(const CaptureRequest &request)
{
    CaptureKind kind = request.kind;
    QRect region = request.region;

    if (kind == CaptureKind::Region || kind == CaptureKind::FreehandRegion
        || kind == CaptureKind::FixedRegion) {
        kind = CaptureKind::Region;
    }

    const bool isChanged = region != m_lastRegion || kind != m_lastKind;
    m_lastRegion = region;
    m_lastKind = kind;

    if (isChanged) {
        save();
        Q_EMIT changed();
    }
}

void CaptureHistory::rememberRegion(const QRect &region, CaptureKind kind)
{
    CaptureRequest request;
    request.kind = kind;
    request.region = region;
    remember(request);
}

void CaptureHistory::clear()
{
    if (m_lastRegion.isEmpty())
        return;
    m_lastRegion = QRect();
    save();
    Q_EMIT changed();
}

void CaptureHistory::load()
{
    QSettings &settings = Config::instance().settings();
    const QRect stored = settings.value(QStringLiteral("capture/lastRegion")).toRect();
    if (stored.isValid() && !stored.isEmpty()) {
        m_lastRegion = stored;
        m_lastKind = captureKindFromId(settings.value(QStringLiteral("capture/lastKind")).toString());
    }
}

void CaptureHistory::save() const
{
    QSettings &settings = Config::instance().settings();
    settings.setValue(QStringLiteral("capture/lastRegion"), m_lastRegion);
    settings.setValue(QStringLiteral("capture/lastKind"), captureKindId(m_lastKind));
    settings.sync();
}

} // namespace imagen
