// SPDX-License-Identifier: MIT
#include "hotkeys/HotkeyAction.h"
#include "hotkeys/HotkeySequence.h"
#include "hotkeys/KeyNames.h"

#include <QKeySequence>
#include <QTest>

using namespace imagen;

class TestHotkeys : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void actionIdentifiersRoundTrip();
    void actionIdentifiersAreUnique();
    void defaultShortcutsAreValid();
    void sequenceParsing();
    void sequenceFormatting();
    void emptySequenceIsInvalid();
    void keyNamesUseTheXkbScheme();
};

void TestHotkeys::actionIdentifiersRoundTrip()
{
    const QList<HotkeyAction> actions = allHotkeyActions();
    QVERIFY(!actions.isEmpty());

    for (HotkeyAction action : actions) {
        const QString id = hotkeyActionId(action);
        QVERIFY(!id.isEmpty());
        QCOMPARE(hotkeyActionFromId(id), action);
    }
}

void TestHotkeys::actionIdentifiersAreUnique()
{
    QSet<QString> identifiers;
    for (HotkeyAction action : allHotkeyActions())
        identifiers.insert(hotkeyActionId(action));

    QCOMPARE(identifiers.size(), allHotkeyActions().size());
}

void TestHotkeys::defaultShortcutsAreValid()
{
    for (HotkeyAction action : allHotkeyActions())
        QVERIFY(!hotkeyActionName(action).isEmpty());

    // The full screen capture is bound to the print key by default.
    const HotkeySequence sequence = defaultHotkeySequence(HotkeyAction::CaptureFullScreen);
    QVERIFY(sequence.isValid());
    QCOMPARE(sequence.toKeySequence(), QKeySequence(Qt::Key_Print));
}

void TestHotkeys::sequenceParsing()
{
    const HotkeySequence sequence = HotkeySequence::fromString(QStringLiteral("Ctrl+Shift+Print"));
    QVERIFY(sequence.isValid());
    QCOMPARE(sequence.toKeySequence(),
             QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_Print));

    // The stored representation uses the portable text format.
    QCOMPARE(HotkeySequence::fromString(sequence.toString()).toString(), sequence.toString());
}

void TestHotkeys::sequenceFormatting()
{
    // QKeySequence normalises the key names, "Delete" becomes "Del".
    QCOMPARE(HotkeySequence::fromString(QStringLiteral("Ctrl+Alt+Delete")).toString(),
             QStringLiteral("Ctrl+Alt+Del"));
    QCOMPARE(HotkeySequence::fromString(QStringLiteral("F5")).toString(), QStringLiteral("F5"));

    // The parsed sequence keeps its meaning.
    QCOMPARE(HotkeySequence::fromString(QStringLiteral("Ctrl+Alt+Del")).toKeySequence(),
             QKeySequence(Qt::ControlModifier | Qt::AltModifier | Qt::Key_Delete));
}

void TestHotkeys::emptySequenceIsInvalid()
{
    QVERIFY(!HotkeySequence::fromString(QString()).isValid());
    QVERIFY(!HotkeySequence::fromString(QStringLiteral("   ")).isValid());
    QVERIFY(!HotkeySequence().isValid());
}

void TestHotkeys::keyNamesUseTheXkbScheme()
{
    QCOMPARE(KeyNames::forKey(QKeySequence(Qt::Key_Print)), QStringLiteral("Print"));
    QCOMPARE(KeyNames::forKey(QKeySequence(Qt::Key_Space)), QStringLiteral("space"));
    QCOMPARE(KeyNames::forKey(QKeySequence(Qt::Key_Escape)), QStringLiteral("Escape"));
    QCOMPARE(KeyNames::forKey(QKeySequence(Qt::Key_A)), QStringLiteral("A"));
    QCOMPARE(KeyNames::forKey(QKeySequence(Qt::Key_F12)), QStringLiteral("F12"));
    QVERIFY(KeyNames::forKey(QKeySequence(Qt::Key_Shift)).isEmpty());
}

QTEST_MAIN(TestHotkeys)
#include "tst_hotkeys.moc"
