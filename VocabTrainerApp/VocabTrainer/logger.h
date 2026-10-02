#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QDateTime>

enum class LogLevel
{
    Info,
    Warning,
    Error,
    Debug,
    Verbose
};

struct LogEntry
{
    // Info for context
    Qt::HANDLE threadId;
    QString threadName;
    quint64 sequence;   // LogFileWriter's ordering number
    QString tag;

    // Actual log message
    QDateTime timestamp;    // Time of logging
    LogLevel level;
    QString message;    // Actual log message
};

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(QObject *parent = nullptr);

    void log(LogLevel level, const QString &tag, const QString &message);

    void info(const QString &tag, const QString &message);

    void warning(const QString &tag, const QString &message);

    void error(const QString &tag, const QString &message);

    void debug(const QString &tag, const QString &message);

    void verbose(const QString &tag, const QString &message);

signals:
    void logEntryCreated(const LogEntry &entry);
};

#endif // LOGGER_H
