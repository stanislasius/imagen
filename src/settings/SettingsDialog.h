// SPDX-License-Identifier: MIT
#pragma once

#include <QDialog>
#include <QHash>

class QCheckBox;
class QComboBox;
class QKeySequenceEdit;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;

namespace imagen {

/**
 * The settings dialog.
 *
 * Every control writes directly into the configuration; the dialog only
 * collects the values and reports that something changed.
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    /// Reloads the controls from the configuration.
    void reload();

Q_SIGNALS:
    /// Emitted after a value was changed.
    void settingsChanged();

private:
    QWidget *createCaptureTab();
    QWidget *createShortcutTab();
    QWidget *createAppearanceTab();
    QWidget *createDiagnosticsTab();

    void applyCaptureTab();
    void applyShortcuts();
    void applyAppearance();

    void browseAutoSaveFolder();

    QComboBox *m_outputTarget = nullptr;
    QSpinBox *m_delay = nullptr;
    QCheckBox *m_includeCursor = nullptr;
    QCheckBox *m_playSound = nullptr;
    QCheckBox *m_showNotification = nullptr;
    QCheckBox *m_alwaysCopy = nullptr;
    QLineEdit *m_autoSaveFolder = nullptr;
    QPushButton *m_autoSaveBrowse = nullptr;
    QLineEdit *m_autoSavePattern = nullptr;
    QComboBox *m_autoSaveFormat = nullptr;
    QLineEdit *m_externalProgram = nullptr;

    QTableWidget *m_shortcutTable = nullptr;

    QComboBox *m_theme = nullptr;
    QComboBox *m_language = nullptr;

    QHash<QString, QKeySequenceEdit *> m_shortcutEditors;
};

} // namespace imagen
