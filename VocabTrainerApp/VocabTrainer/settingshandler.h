#ifndef SETTINGSHANDLER_H
#define SETTINGSHANDLER_H

#include "Logging/logger.h"

#include <QObject>
#include <QString>
#include <QStandardPaths>
#include <QDir>

struct Settings
{
    Settings()
    {
        logEnabled = true;
        logInfoEnabled = true;
        logWarningEnabled = true;
        logErrorEnabled = true;
        logDebugEnabled = true;
        logVerboseEnabled = true;

        const QString logDirectory =QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        if (!QDir().mkpath(logDirectory))
        {
            // TODO: Handle error!
        }
        logFilePath = QDir(logDirectory).filePath(QStringLiteral("vocabtrainer.log"));
    }

    bool logEnabled;
    bool logInfoEnabled;
    bool logWarningEnabled;
    bool logErrorEnabled;
    bool logDebugEnabled;
    bool logVerboseEnabled;

    QString logFilePath;
};

class SettingsHandler : public QObject
{
    Q_OBJECT

public:
    explicit SettingsHandler(Logger &logger, QObject* parent = nullptr);

    // TODO: change settings structure to avoid this repetition
    const Settings& settings() const;

    bool getLogEnabled();
    void setLogEnabled(const bool isEnabled);

    bool getLogInfoEnabled();
    void setLogInfoEnabled(const bool isEnabled);

    bool getLogWarningEnabled();
    void setLogWarningEnabled(const bool isEnabled);

    bool getLogErrorEnabled();
    void setLogErrorEnabled(const bool isEnabled);

    bool getLogDebugEnabled();
    void setLogDebugEnabled(const bool isEnabled);

    bool getLogVerboseEnabled();
    void setLogVerboseEnabled(const bool isEnabled);

    const QString &getLogFilePath();
    void setLogFilePath(const QString &newPath);

public slots:
    void applySettings(const Settings& newSettings);

signals:
    void applySettingsFinished(const Settings& settings);

private:
    Logger &m_logger;
    Settings m_settings;
};

#endif // SETTINGSHANDLER_H
