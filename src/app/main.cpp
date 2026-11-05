// SPDX-License-Identifier: MIT
#include "app/Application.h"

#include "capture/CaptureManager.h"
#include "capture/CaptureTypes.h"
#include "common/AppInfo.h"
#include "common/Logging.h"
#include "common/Platform.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDir>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTimer>

namespace {

/**
 * Name of the local socket that keeps the application single instance.
 */
QString singleInstanceServerName()
{
    return QStringLiteral("%1-%2")
            .arg(QLatin1String(imagen::AppInfo::AppId),
                 QDir::homePath().remove(QLatin1Char('/')).replace(QLatin1Char('/'), QLatin1Char('-')));
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    QCoreApplication::setApplicationName(QLatin1String(imagen::AppInfo::ApplicationName));
    QCoreApplication::setApplicationVersion(imagen::AppInfo::version());
    QCoreApplication::setOrganizationName(QLatin1String(imagen::AppInfo::ApplicationName));
    QCoreApplication::setOrganizationDomain(QStringLiteral("imagen.org"));
    QGuiApplication::setDesktopFileName(QLatin1String(imagen::AppInfo::DesktopFileName));

    imagen::installLoggingHandler();

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "Screenshot capture and annotation"));
    parser.addHelpOption();
    parser.addVersionOption();

    const QCommandLineOption captureOption(
            { QStringLiteral("c"), QStringLiteral("capture") },
            QCoreApplication::translate("main", "Take a capture immediately: <fullscreen|monitor|window|region|last>."),
            QCoreApplication::translate("main", "kind"));
    parser.addOption(captureOption);

    const QCommandLineOption diagnosticsOption(
            QStringLiteral("diagnostics"),
            QCoreApplication::translate("main", "Print the platform diagnostics and exit."));
    parser.addOption(diagnosticsOption);

    parser.process(application);

    if (parser.isSet(diagnosticsOption)) {
        const QList<imagen::CapabilityStatus> report = imagen::Platform::capabilityReport();
        for (const imagen::CapabilityStatus &status : report)
            qInfo().noquote() << status.name << ':' << status.details;
        return 0;
    }

    // Only one instance may run at a time; a second one hands its request
    // over to the running instance and exits.
    const QString serverName = singleInstanceServerName();
    QLocalSocket client;
    client.connectToServer(serverName);
    if (client.waitForConnected(500)) {
        client.write("activate");
        client.waitForBytesWritten(500);
        return 0;
    }

    QLocalServer server;
    server.setSocketOptions(QLocalServer::UserAccessOption);
    if (!server.listen(serverName)) {
        qCWarning(lcApp) << "cannot listen on the single instance socket:" << server.errorString();
    }

    imagen::Application app;
    app.start();

    if (parser.isSet(captureOption)) {
        const QString kind = parser.value(captureOption);
        QTimer::singleShot(250, &app, [&app, kind]() {
            app.handleAction(QStringLiteral("capture.%1").arg(kind));
        });
    }

    return application.exec();
}
