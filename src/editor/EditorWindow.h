// SPDX-License-Identifier: MIT
#pragma once

#include <QImage>
#include <QMainWindow>

class QGraphicsScene;
class QGraphicsView;
class QLabel;

namespace imagen {

/**
 * The image editor window.
 *
 * The canvas shows the image with pan and zoom. The annotation layer is
 * added in the editor milestone; for now the window provides the file
 * operations and the zoom handling every later step builds on.
 */
class EditorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EditorWindow(QWidget *parent = nullptr);

    /// Shows an image in the editor.
    void setImage(const QImage &image);

    /// The image currently being edited.
    QImage image() const { return m_image; }

    /// Writes the image to a file.
    bool saveAs(const QString &fileName);

public Q_SLOTS:
    void zoomIn();
    void zoomOut();
    void zoomOriginal();
    void zoomToFit();

private:
    void updateStatus();

    QGraphicsScene *m_scene = nullptr;
    QGraphicsView *m_view = nullptr;
    QLabel *m_status = nullptr;
    QImage m_image;
};

} // namespace imagen
