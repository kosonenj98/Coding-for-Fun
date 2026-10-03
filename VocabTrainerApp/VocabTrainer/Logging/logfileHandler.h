#ifndef LOGFILEHANDLER_H
#define LOGFILEHANDLER_H

#include "logger.h"
#include "../settingshandler.h"
#include "../errorcode.h"

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
    void writeEntry(const LogEntry &entry);
    void flush();
    void setFilePath(const QString &newFilePath);
    void readAllLogEntries(const LogQuery &query);
    void shutdown();

signals:
    void readAllLogEntriesSucceeded(const LogQuery &query, QList<LogEntry> entries);
    void readAllLogEntriesSucceededPartially(const LogQuery &query, QList<LogEntry> entries, int failedEntriesCount);
    void readAllLogEntriesFailed(ErrorCode code);

private:
    void logInternally(LogLevel level, const QString &message);
    void info(const QString &message);
    void warning(const QString &message);
    void error(const QString &message);
    void debug(const QString &message);
    void verbose(const QString &message);
    bool isLoggingEnabled(const LogEntry &entry);

    SettingsHandler &m_settingsHandler;
    QFile m_logFile;
    quint64 m_sequence = 0;
    QTimer *m_flushTimer;
    int m_entriesSinceFlush = 0;

    static constexpr int FlushInterval = 100;
    static constexpr int FlushTimerIntervalMs = 1000;
};

#endif // LOGFILEHANDLER_H
