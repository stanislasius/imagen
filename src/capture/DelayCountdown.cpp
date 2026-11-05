// SPDX-License-Identifier: MIT
#include "capture/DelayCountdown.h"

#include "common/Logging.h"

#include <QFontMetrics>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QScreen>
#include <QTimerEvent>

namespace imagen {
namespace {

QRect desktopGeometry()
{
    QRect bounds;
    const QList<QScreen *> screens = QGuiApplication::screens();
    for (QScreen *screen : screens)
        bounds = bounds.united(screen->geometry());
    return bounds;
}

} // namespace

DelayCountdown::DelayCountdown(QWidget *parent)
    : OverlayWindow(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::BlankCursor);
}

void DelayCountdown::start(int seconds)
{
    stop();

    if (seconds <= 0) {
        Q_EMIT finished();
        return;
    }

    m_remaining = seconds;
    setGeometry(desktopGeometry());
    show();
    raise();
    activateWindow();
    setFocus(Qt::ActiveWindowFocusReason);
    update();

    m_timerId = startTimer(1000);
    qCInfo(lcCapture) << "countdown started with" << seconds << "seconds";
}

void DelayCountdown::stop()
{
    if (m_timerId) {
        killTimer(m_timerId);
        m_timerId = 0;
    }
    if (isVisible())
        hide();
}

void DelayCountdown::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)
    if (m_timerId == 0)
        return;

    --m_remaining;
    if (m_remaining <= 0) {
        stop();
        Q_EMIT finished();
        return;
    }
    update();
}

void DelayCountdown::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        stop();
        Q_EMIT cancelled();
        return;
    }
    OverlayWindow::keyPressEvent(event);
}

void DelayCountdown::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor(0, 0, 0, 120));

    QFont font = painter.font();
    font.setPointSize(qMax(24, qMin(rect().width(), rect().height()) / 6));
    font.setBold(true);
    painter.setFont(font);

    painter.setPen(Qt::white);
    const QString text = QString::number(qMax(1, m_remaining));
    painter.drawText(rect(), Qt::AlignCenter, text);
}

} // namespace imagen
