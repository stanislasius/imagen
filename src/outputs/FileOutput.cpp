// SPDX-License-Identifier: MIT
#include "outputs/FileOutput.h"

#include "common/Logging.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QImageWriter>

namespace imagen {
namespace {

struct Format {
    const char *suffix;
    const char *description;
};

const Format kFormats[] = {
    { "png", "PNG image (*.png)" },
    { "jpg", "JPEG image (*.jpg *.jpeg)" },
    { "bmp", "BMP image (*.bmp)" },
    { "webp", "WebP image (*.webp)" },
    { "tif", "TIFF image (*.tif *.tiff)" },
    { "gif", "GIF image (*.gif)" },
    { "heic", "HEIF image (*.heic *.heif)" },
    { "avif", "AVIF image (*.avif)" },
    { "jxl", "JPEG XL image (*.jxl)" },
};

} // namespace

FileOutput::FileOutput(QObject *parent)
    : QObject(parent)
{
}

QString FileOutput::fileDialogFilter()
{
    QStringList filters;
    filters << QCoreApplication::translate("FileOutput", "All supported images (*.png *.jpg *.jpeg *.bmp *.webp *.tif *.tiff *.gif *.heic *.heif *.avif *.jxl)");
    for (const Format &format : kFormats)
        filters << QCoreApplication::translate("FileOutput", format.description);
    filters << QCoreApplication::translate("FileOutput", "All files (*)");
    return filters.join(QStringLiteral(";;"));
}

bool FileOutput::write(const QImage &image, const QString &fileName)
{
    if (image.isNull() || fileName.isEmpty())
        return false;

    const QString suffix = QFileInfo(fileName).suffix().toLower();
    QImageWriter writer(fileName);
    if (!suffix.isEmpty())
        writer.setFormat(suffix.toLatin1().constData());

    if (!writer.write(image)) {
        qCWarning(lcOutputs) << "cannot write" << fileName << ":" << writer.errorString();
        return false;
    }

    qCInfo(lcOutputs) << "image written to" << fileName;
    return true;
}

QString FileOutput::save(const QImage &image, const QString &suggestedName)
{
    if (image.isNull())
        return {};

    QString fileName = suggestedName;
    if (fileName.isEmpty()) {
        fileName = QDir(QDir::homePath()).filePath(QStringLiteral("screenshot.png"));
    } else if (QFileInfo(fileName).suffix().isEmpty()) {
        fileName += QStringLiteral(".png");
    }

    const QString selected = QFileDialog::getSaveFileName(nullptr,
                                                          tr("Save image"),
                                                          fileName,
                                                          fileDialogFilter());
    if (selected.isEmpty())
        return {};

    if (!write(image, selected))
        return {};

    return selected;
}

} // namespace imagen
