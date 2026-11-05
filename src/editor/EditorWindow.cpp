// SPDX-License-Identifier: MIT
#include "editor/EditorWindow.h"

#include "common/Logging.h"
#include "outputs/FileOutput.h"

#include <QAction>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QImageReader>
#include <QLabel>
#include <QMenuBar>
#include <QScrollBar>
#include <QStatusBar>
#include <QToolBar>

namespace imagen {

EditorWindow::EditorWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("imagen"));
    resize(900, 700);

    m_scene = new QGraphicsScene(this);
    m_view = new QGraphicsView(m_scene, this);
    m_view->setDragMode(QGraphicsView::ScrollHandDrag);
    m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    m_view->setBackgroundBrush(QBrush(palette().color(QPalette::Mid)));
    setCentralWidget(m_view);

    m_status = new QLabel(this);
    statusBar()->addPermanentWidget(m_status);

    auto *fileMenu = menuBar()->addMenu(tr("&File"));
    auto *openAction = fileMenu->addAction(tr("&Open…"));
    connect(openAction, &QAction::triggered, this, [this]() {
        const QString fileName = QFileDialog::getOpenFileName(this, tr("Open image"));
        if (fileName.isEmpty())
            return;
        QImage loaded(fileName);
        if (loaded.isNull()) {
            statusBar()->showMessage(tr("The image could not be opened."), 5000);
            return;
        }
        setImage(loaded);
    });

    auto *saveAction = fileMenu->addAction(tr("&Save as…"));
    connect(saveAction, &QAction::triggered, this, [this]() {
        const QString fileName = QFileDialog::getSaveFileName(this, tr("Save image"),
                                                              QStringLiteral("image.png"),
                                                              FileOutput::fileDialogFilter());
        if (fileName.isEmpty())
            return;
        if (saveAs(fileName))
            statusBar()->showMessage(tr("Saved %1").arg(fileName), 5000);
    });

    fileMenu->addSeparator();
    auto *closeAction = fileMenu->addAction(tr("&Close"));
    connect(closeAction, &QAction::triggered, this, &QWidget::close);

    auto *viewMenu = menuBar()->addMenu(tr("&View"));
    auto *zoomInAction = viewMenu->addAction(tr("Zoom &in"));
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    connect(zoomInAction, &QAction::triggered, this, &EditorWindow::zoomIn);

    auto *zoomOutAction = viewMenu->addAction(tr("Zoom &out"));
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    connect(zoomOutAction, &QAction::triggered, this, &EditorWindow::zoomOut);

    auto *originalAction = viewMenu->addAction(tr("&Original size"));
    originalAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    connect(originalAction, &QAction::triggered, this, &EditorWindow::zoomOriginal);

    auto *fitAction = viewMenu->addAction(tr("&Fit to window"));
    fitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_9));
    connect(fitAction, &QAction::triggered, this, &EditorWindow::zoomToFit);
}

void EditorWindow::setImage(const QImage &image)
{
    m_image = image;

    m_scene->clear();
    if (!image.isNull()) {
        m_scene->addPixmap(QPixmap::fromImage(image));
        m_scene->setSceneRect(QRectF(QPointF(0, 0), image.size()));
    }

    setWindowTitle(tr("imagen — %1x%2")
                           .arg(image.width())
                           .arg(image.height()));
    updateStatus();
    zoomToFit();
}

bool EditorWindow::saveAs(const QString &fileName)
{
    if (m_image.isNull() || fileName.isEmpty())
        return false;
    return FileOutput::write(m_image, fileName);
}

void EditorWindow::zoomIn()
{
    m_view->scale(1.25, 1.25);
    updateStatus();
}

void EditorWindow::zoomOut()
{
    m_view->scale(0.8, 0.8);
    updateStatus();
}

void EditorWindow::zoomOriginal()
{
    m_view->resetTransform();
    updateStatus();
}

void EditorWindow::zoomToFit()
{
    if (m_image.isNull())
        return;
    m_view->fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
    updateStatus();
}

void EditorWindow::updateStatus()
{
    const int percent = qRound(m_view->transform().m11() * 100.0);
    m_status->setText(tr("%1 x %2 px — %3%")
                              .arg(m_image.width())
                              .arg(m_image.height())
                              .arg(percent));
}

} // namespace imagen
