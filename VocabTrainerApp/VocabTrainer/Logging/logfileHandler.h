#ifndef LOGFILEHANDLER_H
#define LOGFILEHANDLER_H

#include "../Application/settingshandler.h"
#include "../Application/errorcode.h"

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
    void readAllLogEntries(const Log::Query &query);
    void shutdown();

signals:
    void readAllLogEntriesSucceeded(const Log::Query &query, QList<Log::Entry> entries);
    void readAllLogEntriesSucceededPartially(const Log::Query &query, QList<Log::Entry> entries, int failedEntryCount);
    void readAllLogEntriesFailed(ErrorCode code);

private:
    void logInternally(LogLevel level, const QString &message);
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
