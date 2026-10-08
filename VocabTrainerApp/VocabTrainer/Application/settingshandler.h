#ifndef SETTINGSHANDLER_H
#define SETTINGSHANDLER_H

#include "Logging/logger.h"
#include "DataTypes/requests.h"
#include "DataTypes/results.h"

#include <QObject>
#include <QString>

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
    void handleApplyNewSettingsRequest(const ApplyNewSettingsRequest &request);

signals:
    void applyNewSettingsFinished(const ApplyNewSettingsResult &result);

private:
    Logger &m_logger;
    Settings m_settings;
};

#endif // SETTINGSHANDLER_H
