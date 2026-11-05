// SPDX-License-Identifier: MIT
#include "settings/Config.h"

#include <QSettings>
#include <QTemporaryDir>
#include <QTest>

using namespace imagen;

class TestConfig : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void defaultsAreSensible();
    void valuesSurviveARoundTrip();
    void outputTargetRoundTrip();
    void themeRoundTrip();
    void hotkeyRoundTrip();
    void invalidValuesAreClamped();

private:
    QTemporaryDir m_dir;
    QString m_path;
};

void TestConfig::initTestCase()
{
    QVERIFY(m_dir.isValid());
    m_path = m_dir.filePath(QStringLiteral("imagen.ini"));
    Config::setSettingsFilePath(m_path);
}

void TestConfig::cleanupTestCase()
{
    Config::instance().sync();
}

void TestConfig::defaultsAreSensible()
{
    Config &config = Config::instance();

    QCOMPARE(config.captureDelay(), 0);
    QCOMPARE(config.includeCursor(), false);
    QCOMPARE(config.playShutterSound(), true);
    QCOMPARE(config.showNotification(), true);
    QCOMPARE(config.alwaysCopyToClipboard(), true);
    QCOMPARE(config.outputTarget(), OutputTarget::Editor);
    QCOMPARE(config.themeMode(), ThemeMode::System);
    QCOMPARE(config.autoSaveFormat(), QStringLiteral("png"));
    QCOMPARE(config.autoSavePattern(), QStringLiteral("screenshot_%Y%m%d_%H%M%S"));
}

void TestConfig::valuesSurviveARoundTrip()
{
    Config &config = Config::instance();

    config.setCaptureDelay(7);
    config.setIncludeCursor(true);
    config.setPlayShutterSound(false);
    config.setShowNotification(false);
    config.setAlwaysCopyToClipboard(false);
    config.setAutoSaveDirectory(QStringLiteral("/tmp/imagen-test"));
    config.setAutoSavePattern(QStringLiteral("shot_%H%M%S"));
    config.setAutoSaveFormat(QStringLiteral("webp"));
    config.setExternalProgram(QStringLiteral("gimp %f"));
    config.sync();

    QCOMPARE(config.captureDelay(), 7);
    QCOMPARE(config.includeCursor(), true);
    QCOMPARE(config.playShutterSound(), false);
    QCOMPARE(config.showNotification(), false);
    QCOMPARE(config.alwaysCopyToClipboard(), false);
    QCOMPARE(config.autoSaveDirectory(), QStringLiteral("/tmp/imagen-test"));
    QCOMPARE(config.autoSavePattern(), QStringLiteral("shot_%H%M%S"));
    QCOMPARE(config.autoSaveFormat(), QStringLiteral("webp"));
    QCOMPARE(config.externalProgram(), QStringLiteral("gimp %f"));

    // Restore the defaults for the remaining test cases.
    config.setCaptureDelay(0);
    config.setIncludeCursor(false);
    config.setPlayShutterSound(true);
    config.setShowNotification(true);
    config.setAlwaysCopyToClipboard(true);
    config.setAutoSavePattern(QStringLiteral("screenshot_%Y%m%d_%H%M%S"));
    config.setAutoSaveFormat(QStringLiteral("png"));
}

void TestConfig::outputTargetRoundTrip()
{
    Config &config = Config::instance();

    const OutputTarget targets[] = { OutputTarget::Editor, OutputTarget::Clipboard, OutputTarget::File,
                                     OutputTarget::AutoSave, OutputTarget::ExternalProgram,
                                     OutputTarget::OpenFolder };
    for (OutputTarget target : targets) {
        config.setOutputTarget(target);
        QCOMPARE(config.outputTarget(), target);
    }

    config.setOutputTarget(OutputTarget::Editor);
}

void TestConfig::themeRoundTrip()
{
    Config &config = Config::instance();

    config.setThemeMode(ThemeMode::Dark);
    QCOMPARE(config.themeMode(), ThemeMode::Dark);
    config.setThemeMode(ThemeMode::Light);
    QCOMPARE(config.themeMode(), ThemeMode::Light);
    config.setThemeMode(ThemeMode::System);
    QCOMPARE(config.themeMode(), ThemeMode::System);
}

void TestConfig::hotkeyRoundTrip()
{
    Config &config = Config::instance();

    config.setHotkeySequence(QStringLiteral("capture.region"), QStringLiteral("Ctrl+Shift+R"));
    QCOMPARE(config.hotkeySequence(QStringLiteral("capture.region")), QStringLiteral("Ctrl+Shift+R"));

    // An empty sequence removes the binding.
    config.setHotkeySequence(QStringLiteral("capture.region"), QString());
    QVERIFY(config.hotkeySequence(QStringLiteral("capture.region")).isEmpty());
}

void TestConfig::invalidValuesAreClamped()
{
    Config &config = Config::instance();

    config.setCaptureDelay(-5);
    QCOMPARE(config.captureDelay(), 0);

    config.setAutoSaveQuality(500);
    QCOMPARE(config.autoSaveQuality(), 100);

    config.setAutoSaveQuality(0);
    QCOMPARE(config.autoSaveQuality(), 1);

    config.setAutoSaveQuality(90);
}

QTEST_MAIN(TestConfig)
#include "tst_config.moc"
