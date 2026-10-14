#include "settingshandler.h"

#include <QStandardPaths>
#include <QDir>

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
    const QString logDirectory = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);

    if (!QDir().mkpath(logDirectory))
    {
        m_logger.warning(logTag(), QStringLiteral("Failed to create log directory."));
        return;
    }

    m_settings.logFilePath = QDir(logDirectory).filePath(QStringLiteral("vocabtrainer.log"));
    m_logger.verbose(logTag(), QStringLiteral("Constructing SettingsHandler done!"));
}

const Settings::Settings &SettingsHandler::settings() const
{
    return m_settings;
}

void SettingsHandler::handleApplyNewSettingsRequest(const Settings::ApplyNewRequest &request)
{
    Settings::ApplyNewResult result;
    m_logger.debug(logTag(), QStringLiteral("Applying new settings..."));
    m_settings = request.settings;
    m_logger.debug(logTag(), QStringLiteral("Applying new settings succeeded!"));

    // TODO: Check the suggested settings. If not valid, emit failure signal!

    m_logger.debug(logTag(), QStringLiteral("Emitting success signal..."));
    emit applyNewSettingsFinished(result);
}
