// SPDX-License-Identifier: MIT
#include "common/Logging.h"

#include "common/AppInfo.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <QTextStream>

Q_LOGGING_CATEGORY(lcApp, "imagen.app")
Q_LOGGING_CATEGORY(lcCapture, "imagen.capture")
Q_LOGGING_CATEGORY(lcHotkeys, "imagen.hotkeys")
Q_LOGGING_CATEGORY(lcEditor, "imagen.editor")
Q_LOGGING_CATEGORY(lcOutputs, "imagen.outputs")
Q_LOGGING_CATEGORY(lcPlatform, "imagen.platform")

namespace imagen {
namespace {

QFile *g_logFile = nullptr;

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    const QString formatted = QStringLiteral("[%1] %2")
                                      .arg(QDateTime::currentDateTime().toString(Qt::ISODate), message);

    const char *level = nullptr;
    switch (type) {
    case QtDebugMsg:
        level = "debug";
        break;
    case QtInfoMsg:
        level = "info";
        break;
    case QtWarningMsg:
        level = "warn";
        break;
    case QtCriticalMsg:
        level = "crit";
        break;
    case QtFatalMsg:
        level = "fatal";
        break;
    }

    QTextStream err(stderr);
    err << formatted;
    if (context.category)
        err << " [" << context.category << ']';
    err << " (" << level << ')';
    err << Qt::endl;

    if (g_logFile && g_logFile->isOpen()) {
        QTextStream file(g_logFile);
        file << formatted;
        if (context.category)
            file << " [" << context.category << ']';
        file << " (" << level << ')';
        file << Qt::endl;
        g_logFile->flush();
    }

    if (type == QtFatalMsg)
        abort();
}

} // namespace

void installLoggingHandler()
{
    qInstallMessageHandler(messageHandler);

    const QString path = AppInfo::cacheDirPath() + QStringLiteral("/imagen.log");
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        file.close();
        auto *f = new QFile(path);
        if (f->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            delete g_logFile;
            g_logFile = f;
            qCInfo(lcApp) << "logging to" << path;
        }
    }
}

} // namespace imagen
