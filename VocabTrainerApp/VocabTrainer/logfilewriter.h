#ifndef LOGFILEWRITER_H
#define LOGFILEWRITER_H

#include "logger.h"

#include <QObject>
#include <QFile>
#include <QTimer>

class LogFileWriter : public QObject
{
    Q_OBJECT
public:
    explicit LogFileWriter(const QString &filePath, QObject *parent = nullptr);

public slots:
    void initialize();
    void writeEntry(const LogEntry &entry);
    void flush();
    void setFilePath(const QString &newFilePath);
    void shutdown();

private:
    void logInternally(LogLevel level, const QString &message);
    static QString logLevelToString(LogLevel level);

    QFile m_logFile;
    quint64 m_sequence = 0;
    QTimer *m_flushTimer;
    int m_entriesSinceFlush = 0;
    bool m_logFileAvailable = false;

    static constexpr int FlushInterval = 100;
    static constexpr int FlushTimerIntervalMs = 1000;
};

#endif // LOGFILEWRITER_H
