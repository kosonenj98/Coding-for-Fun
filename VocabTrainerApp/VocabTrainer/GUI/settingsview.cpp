#include "settingsview.h"

#include <QGroupBox>
#include <QVBoxLayout>
#include <QFormLayout>

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("SettingsView");
        return tag;
    }
}

SettingsView::SettingsView(Logger &logger, const Settings &settings, QWidget *parent)
    : QWidget{parent}, m_logger(logger)
{
    m_logger.verbose(logTag(), QStringLiteral("Constructing settings view..."));
    m_logCheckBox = new QCheckBox(QStringLiteral("LOG"));
    m_logInfoCheckBox = new QCheckBox(QStringLiteral("INFO"));
    m_logWarningCheckBox = new QCheckBox(QStringLiteral("WARNING"));
    m_logErrorCheckBox = new QCheckBox(QStringLiteral("ERROR"));
    m_logDebugCheckBox = new QCheckBox(QStringLiteral("DEBUG"));
    m_logVerboseCheckBox = new QCheckBox(QStringLiteral("VERBOSE"));

    m_logFilePathEdit = new QLineEdit(settings.logFilePath);

    m_applyButton = new QPushButton(QStringLiteral("Apply"));
    connect(m_applyButton,&QPushButton::clicked, this, &SettingsView::formChangeSettingsRequest);

    auto* loggingGroup = new QGroupBox(QStringLiteral("Logging"));

    auto* loggingLayout = new QVBoxLayout(loggingGroup);
    loggingLayout->addWidget(m_logCheckBox);
    loggingLayout->addWidget(m_logInfoCheckBox);
    loggingLayout->addWidget(m_logWarningCheckBox);
    loggingLayout->addWidget(m_logErrorCheckBox);
    loggingLayout->addWidget(m_logDebugCheckBox);
    loggingLayout->addWidget(m_logVerboseCheckBox);

    auto* fileGroup = new QGroupBox(QStringLiteral("Log file"));
    auto* fileLayout = new QFormLayout(fileGroup);
    fileLayout->addRow(QStringLiteral("File path:"), m_logFilePathEdit);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(loggingGroup);
    layout->addWidget(fileGroup);
    layout->addStretch();
    layout->addWidget(m_applyButton);
    m_logger.verbose(logTag(), QStringLiteral("Constructing settings view done!"));
}

void SettingsView::formChangeSettingsRequest()
{
    m_logger.verbose(logTag(), QStringLiteral("Forming request for changing settings..."));
    Settings newSettings;
    newSettings.logEnabled = m_logCheckBox->isChecked();
    newSettings.logInfoEnabled = m_logInfoCheckBox->isChecked();
    newSettings.logWarningEnabled = m_logWarningCheckBox->isChecked();
    newSettings.logErrorEnabled = m_logErrorCheckBox->isChecked();
    newSettings.logDebugEnabled = m_logDebugCheckBox->isChecked();
    newSettings.logVerboseEnabled = m_logVerboseCheckBox->isChecked();
    newSettings.logFilePath = m_logFilePathEdit->text();
    m_logger.verbose(logTag(), QStringLiteral("Forming request for changing settings done!"));

    m_logger.verbose(logTag(), QStringLiteral("Requesting changing settings..."));
    emit settingsChangeRequested(newSettings);
}

void SettingsView::settingsChangeResponded(const Settings &settings)
{
    m_logger.verbose(logTag(), QStringLiteral("Requesting changing settings done!"));

    updateView(settings);
}

void SettingsView::updateView(const Settings &settings)
{
    m_logger.verbose(logTag(), QStringLiteral("Updating settings view..."));
    m_logCheckBox->setChecked(settings.logEnabled);
    m_logInfoCheckBox->setChecked(settings.logInfoEnabled);
    m_logWarningCheckBox->setChecked(settings.logWarningEnabled);
    m_logErrorCheckBox->setChecked(settings.logErrorEnabled);
    m_logDebugCheckBox->setChecked(settings.logDebugEnabled);
    m_logVerboseCheckBox->setChecked(settings.logVerboseEnabled);
    m_logger.verbose(logTag(), QStringLiteral("Updating settings view done!"));
}
