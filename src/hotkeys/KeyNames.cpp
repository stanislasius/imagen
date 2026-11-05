// SPDX-License-Identifier: MIT
#include "hotkeys/KeyNames.h"

#include <QKeySequence>

namespace imagen::KeyNames {

QString forKey(const QKeySequence &sequence)
{
    if (sequence.isEmpty())
        return {};

    const Qt::Key key = sequence[0].key();

    switch (key) {
    case Qt::Key_Print:
        return QStringLiteral("Print");
    case Qt::Key_Space:
        return QStringLiteral("space");
    case Qt::Key_Escape:
        return QStringLiteral("Escape");
    case Qt::Key_Return:
        return QStringLiteral("Return");
    case Qt::Key_Enter:
        return QStringLiteral("KP_Enter");
    case Qt::Key_Tab:
        return QStringLiteral("Tab");
    case Qt::Key_Backtab:
        return QStringLiteral("ISO_Left_Tab");
    case Qt::Key_Backspace:
        return QStringLiteral("BackSpace");
    case Qt::Key_Delete:
        return QStringLiteral("Delete");
    case Qt::Key_Insert:
        return QStringLiteral("Insert");
    case Qt::Key_Home:
        return QStringLiteral("Home");
    case Qt::Key_End:
        return QStringLiteral("End");
    case Qt::Key_PageUp:
        return QStringLiteral("Page_Up");
    case Qt::Key_PageDown:
        return QStringLiteral("Page_Down");
    case Qt::Key_Left:
        return QStringLiteral("Left");
    case Qt::Key_Right:
        return QStringLiteral("Right");
    case Qt::Key_Up:
        return QStringLiteral("Up");
    case Qt::Key_Down:
        return QStringLiteral("Down");
    case Qt::Key_Pause:
        return QStringLiteral("Pause");
    case Qt::Key_ScrollLock:
        return QStringLiteral("Scroll_Lock");
    case Qt::Key_CapsLock:
        return QStringLiteral("Caps_Lock");
    case Qt::Key_NumLock:
        return QStringLiteral("Num_Lock");
    case Qt::Key_Menu:
        return QStringLiteral("Menu");
    case Qt::Key_Comma:
        return QStringLiteral("comma");
    case Qt::Key_Period:
        return QStringLiteral("period");
    case Qt::Key_Slash:
        return QStringLiteral("slash");
    case Qt::Key_Semicolon:
        return QStringLiteral("semicolon");
    case Qt::Key_Apostrophe:
        return QStringLiteral("apostrophe");
    case Qt::Key_BracketLeft:
        return QStringLiteral("bracketleft");
    case Qt::Key_BracketRight:
        return QStringLiteral("bracketright");
    case Qt::Key_Backslash:
        return QStringLiteral("backslash");
    case Qt::Key_Minus:
        return QStringLiteral("minus");
    case Qt::Key_Equal:
        return QStringLiteral("equal");
    case Qt::Key_QuoteLeft:
        return QStringLiteral("grave");
    case Qt::Key_Plus:
        return QStringLiteral("plus");
    case Qt::Key_Asterisk:
        return QStringLiteral("asterisk");
    case Qt::Key_Question:
        return QStringLiteral("question");
    case Qt::Key_Exclam:
        return QStringLiteral("exclam");
    case Qt::Key_NumberSign:
        return QStringLiteral("numbersign");
    case Qt::Key_Dollar:
        return QStringLiteral("dollar");
    case Qt::Key_Percent:
        return QStringLiteral("percent");
    case Qt::Key_Ampersand:
        return QStringLiteral("ampersand");
    case Qt::Key_ParenLeft:
        return QStringLiteral("parenleft");
    case Qt::Key_ParenRight:
        return QStringLiteral("parenright");
    case Qt::Key_Underscore:
        return QStringLiteral("underscore");
    case Qt::Key_Colon:
        return QStringLiteral("colon");
    case Qt::Key_QuoteDbl:
        return QStringLiteral("quotedbl");
    case Qt::Key_Less:
        return QStringLiteral("less");
    case Qt::Key_Greater:
        return QStringLiteral("greater");
    case Qt::Key_BraceLeft:
        return QStringLiteral("braceleft");
    case Qt::Key_BraceRight:
        return QStringLiteral("braceright");
    case Qt::Key_Bar:
        return QStringLiteral("bar");
    case Qt::Key_AsciiTilde:
        return QStringLiteral("asciitilde");
    case Qt::Key_AsciiCircum:
        return QStringLiteral("asciicircum");
    default:
        break;
    }

    if (key >= Qt::Key_A && key <= Qt::Key_Z)
        return QString(QLatin1Char('A' + (key - Qt::Key_A)));
    if (key >= Qt::Key_0 && key <= Qt::Key_9)
        return QString(QLatin1Char('0' + (key - Qt::Key_0)));
    if (key >= Qt::Key_F1 && key <= Qt::Key_F35)
        return QStringLiteral("F%1").arg(key - Qt::Key_F1 + 1);

    return {};
}

} // namespace imagen::KeyNames
