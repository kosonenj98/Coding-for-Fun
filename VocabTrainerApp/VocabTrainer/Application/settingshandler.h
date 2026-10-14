#ifndef SETTINGSHANDLER_H
#define SETTINGSHANDLER_H

#include "Logging/logger.h"
#include "DataTypes/settings.h"

#include <QObject>
#include <QString>


class SettingsHandler : public QObject
{
    Q_OBJECT

public:
    explicit SettingsHandler(Logger &logger, QObject* parent = nullptr);

    const Settings::Settings& settings() const;

public slots:
    void handleApplyNewSettingsRequest(const Settings::ApplyNewRequest &request);

signals:
    void applyNewSettingsFinished(const Settings::ApplyNewResult &result);

private:
    Logger &m_logger;
    Settings::Settings m_settings;
};

#endif // SETTINGSHANDLER_H
