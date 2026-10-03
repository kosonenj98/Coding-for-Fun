#include "logservice.h"

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("LogService");
        return tag;
    }
}

LogService::LogService(Logger &logger, LogFileHandler &handler, QObject *parent)
    : QObject{parent}, m_logger(logger), m_logFileHandler(handler)
{
    m_logger.verbose(logTag(), QStringLiteral("Initializing log service..."));
    connect(this, &LogService::readEntriesRequested, &m_logFileHandler, &LogFileHandler::readEntries);
    connect(&m_logFileHandler, &LogFileHandler::readEntriesFinished, this, &LogService::readEntriesResponded);
    m_logger.verbose(logTag(), QStringLiteral("Initializing log service done!"));
}

void LogService::executeLogQuery(const LogQuery &query)
{
    m_logger.verbose(logTag(), QStringLiteral("Requesting log entry read..."));
    emit readEntriesRequested(query);
}

void LogService::readEntriesResponded(const LogQuery &query, const QList<LogEntry> &entries)
{
    m_logger.verbose(logTag(), QStringLiteral("Requesting log entry read done!"));
    QList<LogEntry> filteredEntries = handleLogEntries(query, entries);

    m_logger.verbose(logTag(), QStringLiteral("Responding to log entry read request..."));
    emit executeLogQueryFinished(filteredEntries);
}

QList<LogEntry> LogService::handleLogEntries(const LogQuery &query, const QList<LogEntry> &entries)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling log entries..."));

    // Filter entries with query
    QList<LogEntry> filteredEntries = entries;

    m_logger.verbose(logTag(), QStringLiteral("Handling log entries done!"));

    return filteredEntries;
}
