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
    connect(this, &LogService::requestGetAllLogEntries, &m_logFileHandler, &LogFileHandler::readAllLogEntries);
    connect(&m_logFileHandler, &LogFileHandler::readAllLogEntriesSucceeded, this, &LogService::handleGetAllLogEntriesSucceeded);
    connect(&m_logFileHandler, &LogFileHandler::readAllLogEntriesSucceededPartially, this, &LogService::handleGetAllLogEntriesSucceededPartially);
    m_logger.verbose(logTag(), QStringLiteral("Initializing log service done!"));
}

void LogService::queryLog(const LogQuery &query)
{
    m_logger.verbose(logTag(), QStringLiteral("Querying log..."));
    emit requestGetAllLogEntries(query);
}

void LogService::handleGetAllLogEntriesSucceeded(const LogQuery &query, const QList<LogEntry> &entries)
{
    m_logger.verbose(logTag(), QStringLiteral("Reading all log entries succeeded!"));
    QList<LogEntry> filteredEntries = filterLogEntries(query, entries);

    m_logger.verbose(logTag(), QStringLiteral("Emitting log query success signal..."));
    emit queryLogSucceeded(filteredEntries);
}

void LogService::handleGetAllLogEntriesSucceededPartially(const LogQuery &query, const QList<LogEntry> &entries, int failedEntriesCount)
{
    m_logger.verbose(logTag(), QStringLiteral("Reading all log entries succeeded partially. Skipped %1 faulty lines in log file.").arg(QString::number(failedEntriesCount)));
    QList<LogEntry> filteredEntries = filterLogEntries(query, entries);

    m_logger.verbose(logTag(), QStringLiteral("Emitting log query partial success signal..."));
    emit queryLogSucceededPartially(filteredEntries, failedEntriesCount);
}

void LogService::handleGetAllLogEntriesFailed(ErrorCode code)
{
    m_logger.error(logTag(), QStringLiteral("Reading all log entries failed! Reason: '%1'").arg(errorCodeToString(code)));

    m_logger.verbose(logTag(), QStringLiteral("Emitting log query failure signal..."));
    emit queryLogFailed(code);
}

QList<LogEntry> LogService::filterLogEntries(const LogQuery &query, const QList<LogEntry> &entries)
{
    m_logger.verbose(logTag(), QStringLiteral("Filtering log entries..."));

    // Filter entries with query
    QList<LogEntry> filteredEntries = entries;

    m_logger.verbose(logTag(), QStringLiteral("Filtering log entries done!"));
    return filteredEntries;
}
