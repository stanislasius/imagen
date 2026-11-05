// SPDX-License-Identifier: MIT
#include "settings/SettingsDialog.h"

#include "common/AppInfo.h"
#include "common/Logging.h"
#include "common/Platform.h"
#include "hotkeys/HotkeyAction.h"
#include "hotkeys/HotkeyManager.h"
#include "hotkeys/HotkeySequence.h"
#include "settings/Config.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace imagen {
namespace {

QTableWidgetItem *makeItem(const QString &text)
{
    auto *item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}

} // namespace

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("imagen settings"));
    setMinimumWidth(560);

    auto *tabs = new QTabWidget(this);
    tabs->addTab(createCaptureTab(), tr("Capture"));
    tabs->addTab(createShortcutTab(), tr("Shortcuts"));
    tabs->addTab(createAppearanceTab(), tr("Appearance"));
    tabs->addTab(createDiagnosticsTab(), tr("Diagnostics"));

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::accept);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(tabs);
    layout->addWidget(buttons);

    reload();
}

void SettingsDialog::reload()
{
    const Config &config = Config::instance();

    m_outputTarget->setCurrentIndex(static_cast<int>(config.outputTarget()));
    m_delay->setValue(config.captureDelay());
    m_includeCursor->setChecked(config.includeCursor());
    m_playSound->setChecked(config.playShutterSound());
    m_showNotification->setChecked(config.showNotification());
    m_alwaysCopy->setChecked(config.alwaysCopyToClipboard());
    m_autoSaveFolder->setText(config.autoSaveDirectory());
    m_autoSavePattern->setText(config.autoSavePattern());
    m_autoSaveFormat->setCurrentText(config.autoSaveFormat());
    m_externalProgram->setText(config.externalProgram());

    for (HotkeyAction action : allHotkeyActions()) {
        const QString id = hotkeyActionId(action);
        QKeySequenceEdit *editor = m_shortcutEditors.value(id);
        if (!editor)
            continue;
        const QString stored = config.hotkeySequence(id);
        const HotkeySequence sequence = stored.isEmpty() ? defaultHotkeySequence(action)
                                                         : HotkeySequence::fromString(stored);
        editor->setKeySequence(sequence.toKeySequence());
    }

    m_theme->setCurrentIndex(static_cast<int>(config.themeMode()));
    const int languageIndex = m_language->findData(config.language());
    m_language->setCurrentIndex(languageIndex >= 0 ? languageIndex : 0);
}

QWidget *SettingsDialog::createCaptureTab()
{
    auto *page = new QWidget(this);
    auto *form = new QFormLayout(page);

    m_outputTarget = new QComboBox(page);
    for (OutputTarget target : { OutputTarget::Editor, OutputTarget::Clipboard, OutputTarget::File,
                                 OutputTarget::AutoSave, OutputTarget::ExternalProgram,
                                 OutputTarget::OpenFolder }) {
        m_outputTarget->addItem(outputTargetName(target), static_cast<int>(target));
    }
    form->addRow(tr("After a capture:"), m_outputTarget);

    m_delay = new QSpinBox(page);
    m_delay->setRange(0, 60);
    m_delay->setSuffix(tr(" s"));
    form->addRow(tr("Delay before the capture:"), m_delay);

    m_includeCursor = new QCheckBox(tr("Include the mouse cursor"), page);
    form->addRow(QString(), m_includeCursor);

    m_playSound = new QCheckBox(tr("Play the shutter sound"), page);
    form->addRow(QString(), m_playSound);

    m_showNotification = new QCheckBox(tr("Show a desktop notification"), page);
    form->addRow(QString(), m_showNotification);

    m_alwaysCopy = new QCheckBox(tr("Always copy the capture to the clipboard"), page);
    form->addRow(QString(), m_alwaysCopy);

    auto *folderLayout = new QHBoxLayout;
    m_autoSaveFolder = new QLineEdit(page);
    m_autoSaveBrowse = new QPushButton(tr("Browse…"), page);
    folderLayout->addWidget(m_autoSaveFolder);
    folderLayout->addWidget(m_autoSaveBrowse);
    form->addRow(tr("Auto-save folder:"), folderLayout);
    connect(m_autoSaveBrowse, &QPushButton::clicked, this, &SettingsDialog::browseAutoSaveFolder);

    m_autoSavePattern = new QLineEdit(page);
    m_autoSavePattern->setPlaceholderText(QStringLiteral("screenshot_%Y%m%d_%H%M%S"));
    form->addRow(tr("Auto-save file name:"), m_autoSavePattern);

    m_autoSaveFormat = new QComboBox(page);
    for (const char *format : { "png", "jpg", "webp", "tif", "bmp" })
        m_autoSaveFormat->addItem(QString::fromLatin1(format));
    form->addRow(tr("Auto-save format:"), m_autoSaveFormat);

    m_externalProgram = new QLineEdit(page);
    m_externalProgram->setPlaceholderText(QStringLiteral("gimp %f"));
    form->addRow(tr("External program:"), m_externalProgram);

    connect(m_outputTarget, &QComboBox::currentIndexChanged, this, &SettingsDialog::applyCaptureTab);
    connect(m_delay, &QSpinBox::valueChanged, this, &SettingsDialog::applyCaptureTab);
    connect(m_includeCursor, &QCheckBox::toggled, this, &SettingsDialog::applyCaptureTab);
    connect(m_playSound, &QCheckBox::toggled, this, &SettingsDialog::applyCaptureTab);
    connect(m_showNotification, &QCheckBox::toggled, this, &SettingsDialog::applyCaptureTab);
    connect(m_alwaysCopy, &QCheckBox::toggled, this, &SettingsDialog::applyCaptureTab);
    connect(m_autoSaveFolder, &QLineEdit::editingFinished, this, &SettingsDialog::applyCaptureTab);
    connect(m_autoSavePattern, &QLineEdit::editingFinished, this, &SettingsDialog::applyCaptureTab);
    connect(m_autoSaveFormat, &QComboBox::currentTextChanged, this, &SettingsDialog::applyCaptureTab);
    connect(m_externalProgram, &QLineEdit::editingFinished, this, &SettingsDialog::applyCaptureTab);

    return page;
}

QWidget *SettingsDialog::createShortcutTab()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    auto *hint = new QLabel(tr("Global shortcuts are grabbed by the desktop environment. "
                               "Changes take effect immediately."),
                             page);
    hint->setWordWrap(true);
    layout->addWidget(hint);

    m_shortcutTable = new QTableWidget(page);
    m_shortcutTable->setColumnCount(2);
    m_shortcutTable->setHorizontalHeaderLabels({ tr("Action"), tr("Shortcut") });
    m_shortcutTable->verticalHeader()->setVisible(false);
    m_shortcutTable->horizontalHeader()->setStretchLastSection(true);

    const QList<HotkeyAction> actions = allHotkeyActions();
    m_shortcutTable->setRowCount(actions.size());

    int row = 0;
    for (HotkeyAction action : actions) {
        const QString id = hotkeyActionId(action);
        m_shortcutTable->setItem(row, 0, makeItem(hotkeyActionName(action)));

        auto *editor = new QKeySequenceEdit(m_shortcutTable);
        editor->setMaximumSequenceLength(4);
        m_shortcutEditors.insert(id, editor);
        m_shortcutTable->setCellWidget(row, 1, editor);

        connect(editor, &QKeySequenceEdit::keySequenceChanged, this, &SettingsDialog::applyShortcuts);
        ++row;
    }

    layout->addWidget(m_shortcutTable);
    return page;
}

QWidget *SettingsDialog::createAppearanceTab()
{
    auto *page = new QWidget(this);
    auto *form = new QFormLayout(page);

    m_theme = new QComboBox(page);
    for (ThemeMode mode : { ThemeMode::System, ThemeMode::Light, ThemeMode::Dark })
        m_theme->addItem(themeModeName(mode), static_cast<int>(mode));
    form->addRow(tr("Theme:"), m_theme);

    m_language = new QComboBox(page);
    m_language->addItem(tr("System"), QStringLiteral("system"));
    m_language->addItem(QStringLiteral("English"), QStringLiteral("en"));
    m_language->addItem(QStringLiteral("Русский"), QStringLiteral("ru"));
    form->addRow(tr("Language:"), m_language);

    connect(m_theme, &QComboBox::currentIndexChanged, this, &SettingsDialog::applyAppearance);
    connect(m_language, &QComboBox::currentIndexChanged, this, &SettingsDialog::applyAppearance);

    return page;
}

QWidget *SettingsDialog::createDiagnosticsTab()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    auto *table = new QTableWidget(page);
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels({ tr("Capability"), tr("State") });
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setStretchLastSection(true);

    const QList<CapabilityStatus> report = Platform::capabilityReport();
    table->setRowCount(report.size());
    int row = 0;
    for (const CapabilityStatus &status : report) {
        table->setItem(row, 0, makeItem(status.name));
        table->setItem(row, 1, makeItem(status.details));
        ++row;
    }
    table->resizeColumnsToContents();

    layout->addWidget(table);
    return page;
}

void SettingsDialog::browseAutoSaveFolder()
{
    const QString directory = QFileDialog::getExistingDirectory(this,
                                                                 tr("Select the auto-save folder"),
                                                                 m_autoSaveFolder->text());
    if (directory.isEmpty())
        return;
    m_autoSaveFolder->setText(directory);
    applyCaptureTab();
}

void SettingsDialog::applyCaptureTab()
{
    Config &config = Config::instance();

    config.setOutputTarget(static_cast<OutputTarget>(m_outputTarget->currentIndex()));
    config.setCaptureDelay(m_delay->value());
    config.setIncludeCursor(m_includeCursor->isChecked());
    config.setPlayShutterSound(m_playSound->isChecked());
    config.setShowNotification(m_showNotification->isChecked());
    config.setAlwaysCopyToClipboard(m_alwaysCopy->isChecked());
    config.setAutoSaveDirectory(m_autoSaveFolder->text());
    config.setAutoSavePattern(m_autoSavePattern->text());
    config.setAutoSaveFormat(m_autoSaveFormat->currentText());
    config.setExternalProgram(m_externalProgram->text());

    Q_EMIT settingsChanged();
}

void SettingsDialog::applyShortcuts()
{
    Config &config = Config::instance();

    for (auto it = m_shortcutEditors.constBegin(); it != m_shortcutEditors.constEnd(); ++it) {
        const QString sequence = it.value()->keySequence().toString(QKeySequence::PortableText);
        config.setHotkeySequence(it.key(), sequence);
    }

    Q_EMIT settingsChanged();
}

void SettingsDialog::applyAppearance()
{
    Config &config = Config::instance();

    config.setThemeMode(static_cast<ThemeMode>(m_theme->currentIndex()));
    config.setLanguage(m_language->currentData().toString());

    Q_EMIT settingsChanged();
}

} // namespace imagen
