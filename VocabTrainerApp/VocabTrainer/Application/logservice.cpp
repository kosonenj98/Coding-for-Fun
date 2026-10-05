#include "logservice.h"
#include "Logging/logentryformatter.h"

#include <QRegularExpression>

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

void LogService::handleGetAllLogEntriesSucceededPartially(const LogQuery &query, const QList<LogEntry> &entries, int failedEntryCount)
{
    m_logger.verbose(logTag(), QStringLiteral("Reading all log entries succeeded partially. Skipped %1 faulty lines in log file.").arg(QString::number(failedEntryCount)));
    QList<LogEntry> filteredEntries = filterLogEntries(query, entries);

    m_logger.verbose(logTag(), QStringLiteral("Emitting log query partial success signal..."));
    emit queryLogSucceededPartially(filteredEntries, failedEntryCount);
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
    QList<LogEntry> filteredEntries;
    int maxEntryCount = query.maxEntryCount;
    if (maxEntryCount > 0)
    {
        filteredEntries.reserve(maxEntryCount);
    }
    else
    {
        filteredEntries.reserve(entries.size());
    }
    LogEntrySelectionOrder order = query.entryOrder;
    const QString &querySearchPattern = query.searchPattern;
    bool queryRegEx = query.useRegularExpression;
    bool queryEntireEntry = query.searchEntireEntry;
    bool querySearchPatternIsValid = false;
    const QRegularExpression regex(query.searchPattern, QRegularExpression::CaseInsensitiveOption);
    if (queryRegEx)
    {
        querySearchPatternIsValid = regex.isValid();
    }
    if (queryRegEx && !querySearchPatternIsValid)
    {
        m_logger.warning(logTag(), QStringLiteral("Invalid search pattern in log query: '%1'. Nothing matches...").arg(query.searchPattern));
    }
    if (queryRegEx && querySearchPattern.isEmpty())
    {
        m_logger.warning(logTag(), QStringLiteral("Empty regEx in log query. Everything matches..."));
    }
    QDateTime from = query.from;
    QDateTime to = query.to;
    bool queryFrom = !from.isNull();
    bool queryTo = !to.isNull();

    for (const LogEntry &entry : entries)
    {
        // Construct text that is compared to query search pattern
        QString matchCandidate = entry.message;
        if (queryEntireEntry)
        {
            // Formulate entry as it is written in log
            matchCandidate = LogEntryFormatter::logFileFormat(entry);
        }

        // Check if search pattern matches
        if (queryRegEx)
        {
            if (!regex.match(matchCandidate).hasMatch())
            {
                // No match
                continue;
            }
        }
        else if (!matchCandidate.contains(querySearchPattern))
        {
            // No match
            continue;
        }

        // Check if entry happened after 'from'
        if (queryFrom && entry.timestamp < from)
        {
            continue;
        }

        // Check if entry happened after 'to'
        if (queryTo && entry.timestamp > to)
        {
            continue;
        }

        // Entry matches with query
        filteredEntries.append(entry);
    }

    int skippedEntryCount = entries.count() - filteredEntries.count();

    m_logger.verbose(logTag(), QStringLiteral("Filtering log entries done!"));
    if (skippedEntryCount > 0)
    {
        m_logger.verbose(logTag(), QStringLiteral("Skipped %1 entries.").arg(QString::number(skippedEntryCount)));
    }

    // Truncate filtered entries list if needed
    if (maxEntryCount > 0 && filteredEntries.size() > maxEntryCount)
    {
        m_logger.verbose(logTag(), QStringLiteral("Truncating filtered entry list..."));
        switch (order)
        {
        case LogEntrySelectionOrder::Newest:
        {
            m_logger.verbose(logTag(), QStringLiteral("Getting %1 newest entries...").arg(QString::number(maxEntryCount)));
            filteredEntries = filteredEntries.mid(qMax(0, filteredEntries.size() - maxEntryCount));
            m_logger.verbose(logTag(), QStringLiteral("Getting %1 newest entries done!").arg(QString::number(maxEntryCount)));
            break;
        }

        case LogEntrySelectionOrder::Oldest:
        {
            m_logger.verbose(logTag(), QStringLiteral("Getting %1 oldest entries...").arg(QString::number(maxEntryCount)));
            filteredEntries = filteredEntries.mid(0, maxEntryCount);
            m_logger.verbose(logTag(), QStringLiteral("Getting %1 oldest entries...").arg(QString::number(maxEntryCount)));
            break;
        }
        }
        m_logger.verbose(logTag(), QStringLiteral("Truncating filtered entry list done!"));
    }

    return filteredEntries;
}
