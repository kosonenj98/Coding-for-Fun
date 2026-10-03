#ifndef LOGFILEHANDLER_H
#define LOGFILEHANDLER_H

#include "logger.h"

#include <QObject>
#include <QFile>
#include <QTimer>

class LogFileHandler : public QObject
{
    Q_OBJECT
public:
    explicit LogFileHandler(const QString &filePath, QObject *parent = nullptr);

public slots:
    void initialize();
    void writeEntry(const LogEntry &entry);
    void flush();
    void setFilePath(const QString &newFilePath);
    void readEntries(const LogQuery &query);
    void shutdown();

signals:
    void readEntriesFinished(const LogQuery &query, QList<LogEntry> entries);

private:
    void logInternally(LogLevel level, const QString &message);
    void info(const QString &message);
    void warning(const QString &message);
    void error(const QString &message);
    void debug(const QString &message);
    void verbose(const QString &message);

    QFile m_logFile;
    quint64 m_sequence = 0;
    QTimer *m_flushTimer;
    int m_entriesSinceFlush = 0;
    bool m_logFileAvailable = false;

    static constexpr int FlushInterval = 100;
    static constexpr int FlushTimerIntervalMs = 1000;
};

#endif // LOGFILEHANDLER_H
