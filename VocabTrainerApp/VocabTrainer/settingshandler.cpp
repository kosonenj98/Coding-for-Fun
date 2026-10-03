#include "settingshandler.h"

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("SettingsHandler");
        return tag;
    }
}

SettingsHandler::SettingsHandler(Logger &logger, QObject *parent)
    : QObject(parent), m_logger(logger)
{
    m_logger.verbose(logTag(), QStringLiteral("Constructing SettingsHandler..."));
    m_logger.verbose(logTag(), QStringLiteral("Constructing SettingsHandler done!"));
}

const Settings &SettingsHandler::settings() const
{
    return m_settings;
}

bool SettingsHandler::getLogEnabled()
{
    return m_settings.logEnabled;
}

void SettingsHandler::setLogEnabled(const bool isEnabled)
{
    bool wasEnabled = m_settings.logEnabled;
    m_logger.debug(logTag(), QStringLiteral("Trying to set logEnabled from '%1' to '%2'...").arg(QString::number(wasEnabled), QString::number(isEnabled)));
    if (wasEnabled != isEnabled)
    {
        m_settings.logEnabled = isEnabled;
        m_logger.debug(logTag(), QStringLiteral("New value for logEnabled: '%2'!").arg(QString::number(isEnabled)));
        return;
    }

    m_logger.debug(logTag(), QStringLiteral("No change needed for logEnabled. Skipping..."));
}

bool SettingsHandler::getLogInfoEnabled()
{
    return m_settings.logInfoEnabled;
}

void SettingsHandler::setLogInfoEnabled(const bool isEnabled)
{
    bool wasEnabled = m_settings.logInfoEnabled;
    m_logger.debug(logTag(), QStringLiteral("Trying to set logInfoEnabled from '%1' to '%2'...").arg(QString::number(wasEnabled), QString::number(isEnabled)));
    if (wasEnabled != isEnabled)
    {
        m_settings.logInfoEnabled = isEnabled;
        m_logger.debug(logTag(), QStringLiteral("New value for logInfoEnabled: '%2'!").arg(QString::number(isEnabled)));
        return;
    }

    m_logger.debug(logTag(), QStringLiteral("No change needed for logInfoEnabled. Skipping..."));
}

bool SettingsHandler::getLogWarningEnabled()
{
    return m_settings.logWarningEnabled;
}

void SettingsHandler::setLogWarningEnabled(const bool isEnabled)
{
    bool wasEnabled = m_settings.logWarningEnabled;
    m_logger.debug(logTag(), QStringLiteral("Trying to set logWarningEnabled from '%1' to '%2'...").arg(QString::number(wasEnabled), QString::number(isEnabled)));
    if (wasEnabled != isEnabled)
    {
        m_settings.logWarningEnabled = isEnabled;
        m_logger.debug(logTag(), QStringLiteral("New value for logWarningEnabled: '%2'!").arg(QString::number(isEnabled)));
        return;
    }

    m_logger.debug(logTag(), QStringLiteral("No change needed for logWarningEnabled. Skipping..."));
}

bool SettingsHandler::getLogErrorEnabled()
{
    return m_settings.logErrorEnabled;
}

void SettingsHandler::setLogErrorEnabled(const bool isEnabled)
{
    bool wasEnabled = m_settings.logErrorEnabled;
    m_logger.debug(logTag(), QStringLiteral("Trying to set logErrorEnabled from '%1' to '%2'...").arg(QString::number(wasEnabled), QString::number(isEnabled)));
    if (wasEnabled != isEnabled)
    {
        m_settings.logErrorEnabled = isEnabled;
        m_logger.debug(logTag(), QStringLiteral("New value for logErrorEnabled: '%2'!").arg(QString::number(isEnabled)));
        return;
    }

    m_logger.debug(logTag(), QStringLiteral("No change needed for logErrorEnabled. Skipping..."));
}

bool SettingsHandler::getLogDebugEnabled()
{
    return m_settings.logDebugEnabled;
}

void SettingsHandler::setLogDebugEnabled(const bool isEnabled)
{
    bool wasEnabled = m_settings.logDebugEnabled;
    m_logger.debug(logTag(), QStringLiteral("Trying to set logDebugEnabled from '%1' to '%2'...").arg(QString::number(wasEnabled), QString::number(isEnabled)));
    if (wasEnabled != isEnabled)
    {
        m_settings.logDebugEnabled = isEnabled;
        m_logger.debug(logTag(), QStringLiteral("New value for logDebugEnabled: '%2'!").arg(QString::number(isEnabled)));
        return;
    }

    m_logger.debug(logTag(), QStringLiteral("No change needed for logDebugEnabled. Skipping..."));
}

bool SettingsHandler::getLogVerboseEnabled()
{
    return m_settings.logVerboseEnabled;
}

void SettingsHandler::setLogVerboseEnabled(const bool isEnabled)
{
    bool wasEnabled = m_settings.logVerboseEnabled;
    m_logger.debug(logTag(), QStringLiteral("Trying to set logVerboseEnabled from '%1' to '%2'...").arg(QString::number(wasEnabled), QString::number(isEnabled)));
    if (wasEnabled != isEnabled)
    {
        m_settings.logVerboseEnabled = isEnabled;
        m_logger.debug(logTag(), QStringLiteral("New value for logVerboseEnabled: '%2'!").arg(QString::number(isEnabled)));
        return;
    }

    m_logger.debug(logTag(), QStringLiteral("No change needed for logVerboseEnabled. Skipping..."));
}

const QString &SettingsHandler::getLogFilePath()
{
    return m_settings.logFilePath;
}

void SettingsHandler::setLogFilePath(const QString &newPath)
{
    const QString &oldPath = m_settings.logFilePath;
    m_logger.debug(logTag(), QStringLiteral("Trying to set logFilePath from '%1' to '%2'...").arg(oldPath, newPath));
    if (oldPath != newPath)
    {
        m_settings.logFilePath = newPath;
        m_logger.debug(logTag(), QStringLiteral("New value for logFilePath: '%2'!").arg(newPath));
        return;
    }

    m_logger.debug(logTag(), QStringLiteral("No change needed for logFilePath. Skipping..."));
}

void SettingsHandler::applySettings(const Settings &newSettings)
{
    m_logger.debug(logTag(), QStringLiteral("Applying new settings..."));
    m_settings = newSettings;
    m_logger.debug(logTag(), QStringLiteral("Applying new settings done!"));

    m_logger.debug(logTag(), QStringLiteral("Responding to apply settings change request..."));
    emit applySettingsFinished(m_settings);
}
