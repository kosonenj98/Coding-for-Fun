#ifndef LOGFILEHANDLER_H
#define LOGFILEHANDLER_H

#include "Application/settingshandler.h"

#include <QObject>
#include <QFile>
#include <QTimer>

class LogFileHandler : public QObject
{
    Q_OBJECT
public:
    explicit LogFileHandler(SettingsHandler &handler, QObject *parent = nullptr);

public slots:
    void initialize();
    void writeEntry(const Log::Entry &entry);
    void flush();
    void setFilePath(const QString &newFilePath);
    void handleGetAllLogEntriesRequest(const GetAllLogEntriesRequest &request);
    void shutdown();

signals:
    void getAllLogEntriesFinished(const GetAllLogEntriesResult &result);

private:
    const ReadAllResult readAll(const QString &filePath);

    void logInternally(LogLevel::Level level, const QString &message);
    void info(const QString &message);
    void warning(const QString &message);
    void error(const QString &message);
    void debug(const QString &message);
    void verbose(const QString &message);
    bool isLoggingEnabled(const Log::Entry &entry);

    SettingsHandler &m_settingsHandler;
    QFile m_logFile;
    quint64 m_sequence = 0;
    QTimer *m_flushTimer;
    int m_entriesSinceFlush = 0;

    static constexpr int FlushInterval = 100;
    static constexpr int FlushTimerIntervalMs = 1000;
};

#endif // LOGFILEHANDLER_H
