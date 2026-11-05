// SPDX-License-Identifier: MIT
#include "hotkeys/HotkeySequence.h"

namespace imagen {

HotkeySequence::HotkeySequence(QKeySequence sequence)
    : m_sequence(std::move(sequence))
{
}

HotkeySequence HotkeySequence::fromString(const QString &text)
{
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty())
        return {};

    // Portable text keeps the format stable across platforms.
    return HotkeySequence(QKeySequence::fromString(trimmed, QKeySequence::PortableText));
}

QString HotkeySequence::toString() const
{
    return m_sequence.toString(QKeySequence::PortableText);
}

QString HotkeySequence::displayText() const
{
    return m_sequence.toString(QKeySequence::NativeText);
}

bool HotkeySequence::matches(const QKeySequence &other) const
{
    if (!isValid() || other.isEmpty())
        return false;
    return m_sequence == other;
}

} // namespace imagen
