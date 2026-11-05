// SPDX-License-Identifier: MIT
#pragma once

#include <QString>

class QKeySequence;

namespace imagen {

/**
 * Key names in the XKB naming scheme.
 *
 * Both the GlobalShortcuts portal and the X11 backend use these names, so
 * a shortcut is described identically no matter which provider binds it.
 */
namespace KeyNames {

/// The XKB name of a key, empty when the key has no representation.
QString forKey(const QKeySequence &sequence);

} // namespace KeyNames
} // namespace imagen
