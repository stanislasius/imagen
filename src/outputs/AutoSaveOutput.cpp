// SPDX-License-Identifier: MIT
#include "outputs/AutoSaveOutput.h"

#include "common/Logging.h"
#include "outputs/FileOutput.h"
#include "settings/Config.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>

namespace imagen {
namespace {

/**
 * Expands a strftime-like pattern.
 *
 * Supported specifiers are %Y, %y, %m, %d, %H, %M, %S, %j and %%. Every
 * other character is copied verbatim.
 */
QString expandPattern(const QString &pattern, const QDateTime &dateTime)
{
    QString result;
    result.reserve(pattern.size() + 8);

    for (int index = 0; index < pattern.size(); ++index) {
        const QChar character = pattern.at(index);
        if (character != QLatin1Char('%') || index + 1 >= pattern.size()) {
            result += character;
            continue;
        }

        const QChar specifier = pattern.at(++index);
        switch (specifier.toLatin1()) {
        case 'Y':
            result += dateTime.toString(QStringLiteral("yyyy"));
            break;
        case 'y':
            result += dateTime.toString(QStringLiteral("yy"));
            break;
        case 'm':
            result += dateTime.toString(QStringLiteral("MM"));
            break;
        case 'd':
            result += dateTime.toString(QStringLiteral("dd"));
            break;
        case 'H':
            result += dateTime.toString(QStringLiteral("HH"));
            break;
        case 'M':
            result += dateTime.toString(QStringLiteral("mm"));
            break;
        case 'S':
            result += dateTime.toString(QStringLiteral("ss"));
            break;
        case 'j':
            result += dateTime.toString(QStringLiteral("ddd"));
            break;
        case '%':
            result += QLatin1Char('%');
            break;
        default:
            result += QLatin1Char('%');
            result += specifier;
            break;
        }
    }

    return result;
}

} // namespace

AutoSaveOutput::AutoSaveOutput(QObject *parent)
    : QObject(parent)
{
    const Config &config = Config::instance();
    m_directory = config.autoSaveDirectory();
    m_pattern = config.autoSavePattern();
    m_format = config.autoSaveFormat();
}

QString AutoSaveOutput::directory() const
{
    return m_directory;
}

QString AutoSaveOutput::nextFileName() const
{
    const QString stamp = expandPattern(m_pattern, QDateTime::currentDateTime());
    const QString format = m_format.isEmpty() ? QStringLiteral("png") : m_format;
    return QDir(m_directory).filePath(QStringLiteral("%1.%2").arg(stamp, format));
}

QString AutoSaveOutput::save(const QImage &image)
{
    if (image.isNull())
        return {};

    QDir directory(m_directory);
    if (!directory.exists() && !QDir().mkpath(m_directory)) {
        qCWarning(lcOutputs) << "cannot create the auto-save folder" << m_directory;
        return {};
    }

    // Avoid overwriting an existing file by appending a counter.
    QString fileName = nextFileName();
    int counter = 1;
    while (QFileInfo::exists(fileName)) {
        const QFileInfo info(fileName);
        fileName = info.path() + QLatin1Char('/') + info.completeBaseName()
                + QStringLiteral("_%1.%2").arg(counter++).arg(info.suffix());
    }

    if (!FileOutput::write(image, fileName))
        return {};

    return fileName;
}

} // namespace imagen
