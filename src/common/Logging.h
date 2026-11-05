// SPDX-License-Identifier: MIT
#pragma once

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(lcApp)
Q_DECLARE_LOGGING_CATEGORY(lcCapture)
Q_DECLARE_LOGGING_CATEGORY(lcHotkeys)
Q_DECLARE_LOGGING_CATEGORY(lcEditor)
Q_DECLARE_LOGGING_CATEGORY(lcOutputs)
Q_DECLARE_LOGGING_CATEGORY(lcPlatform)

namespace imagen {

/**
 * Installs the global Qt message pattern and creates the log directory.
 *
 * Safe to call more than once. Logs are written to stderr; when a writable
 * cache directory is available they are additionally mirrored into
 * <cache>/imagen.log which is what the diagnostics dialog points at.
 */
void installLoggingHandler();

} // namespace imagen
