// SPDX-License-Identifier: MIT
#include "outputs/AutoSaveOutput.h"
#include "outputs/FileOutput.h"
#include "settings/Config.h"

#include <QDir>
#include <QImage>
#include <QTemporaryDir>
#include <QTest>

using namespace imagen;

class TestOutputs : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void writeImage();
    void writeUnknownFormatFails();
    void autoSaveUsesThePattern();
    void autoSaveDoesNotOverwrite();
};

void TestOutputs::writeImage()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(8, 4, QImage::Format_RGB32);
    image.fill(Qt::red);

    const QString fileName = dir.filePath(QStringLiteral("out.png"));
    QVERIFY(FileOutput::write(image, fileName));
    QVERIFY(QFileInfo::exists(fileName));

    QImage loaded(fileName);
    QCOMPARE(loaded.size(), image.size());
}

void TestOutputs::writeUnknownFormatFails()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QImage image(4, 4, QImage::Format_RGB32);
    image.fill(Qt::blue);

    const QString fileName = dir.filePath(QStringLiteral("out.thisformatdoesnotexist"));
    QVERIFY(!FileOutput::write(image, fileName));
}

void TestOutputs::autoSaveUsesThePattern()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    Config::setSettingsFilePath(dir.filePath(QStringLiteral("imagen.ini")));
    Config &config = Config::instance();
    config.setAutoSaveDirectory(dir.path());
    config.setAutoSavePattern(QStringLiteral("shot_%Y"));
    config.setAutoSaveFormat(QStringLiteral("png"));

    AutoSaveOutput output;
    QImage image(4, 4, QImage::Format_RGB32);
    image.fill(Qt::green);

    const QString fileName = output.save(image);
    QVERIFY(!fileName.isEmpty());
    QVERIFY(QFileInfo::exists(fileName));
    QVERIFY(fileName.endsWith(QStringLiteral(".png")));
    QVERIFY(QFileInfo(fileName).fileName().startsWith(QStringLiteral("shot_")));
}

void TestOutputs::autoSaveDoesNotOverwrite()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    Config::setSettingsFilePath(dir.filePath(QStringLiteral("imagen.ini")));
    Config &config = Config::instance();
    config.setAutoSaveDirectory(dir.path());
    config.setAutoSavePattern(QStringLiteral("fixed"));
    config.setAutoSaveFormat(QStringLiteral("png"));

    AutoSaveOutput output;
    QImage image(4, 4, QImage::Format_RGB32);
    image.fill(Qt::black);

    const QString first = output.save(image);
    const QString second = output.save(image);

    QVERIFY(!first.isEmpty());
    QVERIFY(!second.isEmpty());
    QVERIFY(first != second);
    QVERIFY(QFileInfo::exists(first));
    QVERIFY(QFileInfo::exists(second));
}

QTEST_MAIN(TestOutputs)
#include "tst_outputs.moc"
