// SPDX-License-Identifier: MIT
#include "common/ScreenGeometry.h"

#include <QTest>

using namespace imagen;

class TestGeometry : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void singleScreen();
    void twoScreensSideBySide();
    void negativeOriginIsNormalised();
    void mixedDevicePixelRatio();
    void logicalToDeviceMapping();
    void deviceToLogicalMapping();
    void deviceRectToImageScaling();
    void logicalRectToImageScaling();
    void emptyDesktop();
};

void TestGeometry::singleScreen()
{
    const VirtualDesktop desktop = buildVirtualDesktop({ ScreenDescriptor{ QRect(0, 0, 1920, 1080), 1.0 } });

    QCOMPARE(desktop.screens.size(), 1);
    QCOMPARE(desktop.logicalSize(), QSize(1920, 1080));
    QCOMPARE(desktop.devicePixelSize(), QSize(1920, 1080));
    QCOMPARE(desktop.screens.first().logicalRect, QRect(0, 0, 1920, 1080));
    QCOMPARE(desktop.screens.first().deviceRect, QRect(0, 0, 1920, 1080));
}

void TestGeometry::twoScreensSideBySide()
{
    const VirtualDesktop desktop = buildVirtualDesktop({
        ScreenDescriptor{ QRect(0, 0, 1920, 1080), 1.0 },
        ScreenDescriptor{ QRect(1920, 0, 1280, 1024), 1.0 },
    });

    QCOMPARE(desktop.logicalSize(), QSize(3200, 1080));
    QCOMPARE(desktop.devicePixelSize(), QSize(3200, 1080));
    QCOMPARE(desktop.screens.at(1).deviceRect, QRect(1920, 0, 1280, 1024));
}

void TestGeometry::negativeOriginIsNormalised()
{
    // A monitor to the left of the primary one has a negative X coordinate.
    const VirtualDesktop desktop = buildVirtualDesktop({
        ScreenDescriptor{ QRect(0, 0, 1920, 1080), 1.0 },
        ScreenDescriptor{ QRect(-1280, 0, 1280, 1024), 1.0 },
    });

    QCOMPARE(desktop.logicalBounds(), QRect(0, 0, 3200, 1080));
    QCOMPARE(desktop.logicalSize(), QSize(3200, 1080));

    // The screens are ordered by position, so the left monitor comes first
    // and owns the origin of the desktop.
    QCOMPARE(desktop.screens.at(0).logicalRect, QRect(0, 0, 1280, 1024));
    QCOMPARE(desktop.screens.at(0).deviceRect, QRect(0, 0, 1280, 1024));
    QCOMPARE(desktop.screens.at(1).logicalRect, QRect(1280, 0, 1920, 1080));
    QCOMPARE(desktop.screens.at(1).deviceRect, QRect(1280, 0, 1920, 1080));
}

void TestGeometry::mixedDevicePixelRatio()
{
    const VirtualDesktop desktop = buildVirtualDesktop({
        ScreenDescriptor{ QRect(0, 0, 1920, 1080), 1.0 },
        ScreenDescriptor{ QRect(1920, 0, 1920, 1080), 1.5 },
    });

    QCOMPARE(desktop.devicePixelSize(), QSize(1920 + 2880, 1620));
    QCOMPARE(desktop.screens.at(0).deviceRect, QRect(0, 0, 1920, 1080));
    QCOMPARE(desktop.screens.at(1).deviceRect, QRect(1920, 0, 2880, 1620));
}

void TestGeometry::logicalToDeviceMapping()
{
    const VirtualDesktop desktop = buildVirtualDesktop({
        ScreenDescriptor{ QRect(0, 0, 1920, 1080), 2.0 },
    });

    const QRect device = mapLogicalToDevice(desktop, QRect(100, 50, 200, 100));
    QCOMPARE(device, QRect(200, 100, 400, 200));
}

void TestGeometry::deviceToLogicalMapping()
{
    const VirtualDesktop desktop = buildVirtualDesktop({
        ScreenDescriptor{ QRect(0, 0, 1920, 1080), 2.0 },
    });

    const QRect logical = mapDeviceToLogical(desktop, QRect(200, 100, 400, 200));
    QCOMPARE(logical, QRect(100, 50, 200, 100));
}

void TestGeometry::deviceRectToImageScaling()
{
    // The portal may return an image that is smaller than the desktop.
    const QRect crop = mapDeviceRectToImage(QRect(1920, 0, 1280, 1024),
                                            QSize(3840, 2160),
                                            QSize(1920, 1080));
    QCOMPARE(crop, QRect(960, 0, 640, 512));
}

void TestGeometry::logicalRectToImageScaling()
{
    const QRect bounds(0, 0, 3200, 1080);
    const QRect crop = mapLogicalRectToImage(QRect(1920, 0, 1280, 1080), bounds, QSize(1600, 540));

    QCOMPARE(crop, QRect(960, 0, 640, 540));
}

void TestGeometry::emptyDesktop()
{
    const VirtualDesktop desktop = buildVirtualDesktop(QList<ScreenDescriptor>());
    QVERIFY(desktop.screens.isEmpty());
    QCOMPARE(desktop.devicePixelSize(), QSize(0, 0));

    // Without a known desktop size there is nothing to map onto.
    QVERIFY(mapDeviceRectToImage(QRect(0, 0, 10, 10), QSize(0, 0), QSize(10, 10)).isEmpty());
    QVERIFY(mapLogicalRectToImage(QRect(0, 0, 10, 10), QRect(0, 0, 0, 0), QSize(10, 10)).isEmpty());
}

QTEST_MAIN(TestGeometry)
#include "tst_geometry.moc"
