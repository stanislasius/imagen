// SPDX-License-Identifier: MIT
#pragma once

#include "overlays/OverlayWindow.h"

namespace imagen {

/**
 * Fullscreen countdown shown before a delayed capture starts.
 *
 * The overlay covers the whole virtual desktop and counts down the
 * configured delay, giving the user time to arrange the screen.
 */
class DelayCountdown : public OverlayWindow
{
    Q_OBJECT

public:
    explicit DelayCountdown(QWidget *parent = nullptr);

    /// Starts the countdown. A delay of zero finishes immediately.
    void start(int seconds);

    /// Aborts the countdown.
    void stop();

    /// Whether the countdown is currently running.
    bool isRunning() const { return m_remaining > 0; }

Q_SIGNALS:
    /// Emitted when the countdown reached zero.
    void finished();
    /// Emitted when the user aborted the countdown.
    void cancelled();

protected:
    void timerEvent(QTimerEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    int m_remaining = 0;
    int m_timerId = 0;
};

} // namespace imagen
