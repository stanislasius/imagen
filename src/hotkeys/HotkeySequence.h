// SPDX-License-Identifier: MIT
#pragma once

#include <QKeySequence>
#include <QString>

namespace imagen {

/**
 * A keyboard shortcut.
 *
 * The class adds a stable, human readable representation on top of
 * QKeySequence. The stored form ("Ctrl+Shift+PrtSc") is independent of
 * the platform and therefore safe to write into the settings file.
 */
class HotkeySequence
{
public:
    HotkeySequence() = default;
    explicit HotkeySequence(QKeySequence sequence);

    /// Parses a shortcut from its stored representation.
    static HotkeySequence fromString(const QString &text);

    /// The stored representation, e.g. "Ctrl+Shift+PrtSc".
    QString toString() const;

    /// The representation shown to the user.
    QString displayText() const;

    /// Whether the shortcut contains at least one key.
    bool isValid() const { return !m_sequence.isEmpty(); }

    /// The wrapped key sequence.
    QKeySequence toKeySequence() const { return m_sequence; }

    /// Whether the given key event matches this shortcut.
    bool matches(const QKeySequence &other) const;

    /// Whether two shortcuts are equal.
    bool operator==(const HotkeySequence &other) const { return m_sequence == other.m_sequence; }
    bool operator!=(const HotkeySequence &other) const { return !(*this == other); }

private:
    QKeySequence m_sequence;
};

} // namespace imagen
