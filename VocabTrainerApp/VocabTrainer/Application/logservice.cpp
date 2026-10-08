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

using namespace Log;

LogService::LogService(Logger &logger, QObject *parent)
    : QObject{parent}, m_logger(logger)
{
    m_logger.verbose(logTag(), QStringLiteral("Initializing log service..."));
    m_logger.verbose(logTag(), QStringLiteral("Initializing log service done!"));
}

void LogService::queryLog(const LogQueryRequest &request)
{
    GetAllLogEntriesRequest getAllLogEntriesRequest;
    getAllLogEntriesRequest.request = request;

    emit requestGetAllLogEntries(getAllLogEntriesRequest);
}

void LogService::handleGetAllLogEntriesFinished(const GetAllLogEntriesResult &result)
{
    LogQueryResult logQueryResult;
    if (result.code != ErrorCode::Success)
    {
        logQueryResult.code = result.code;
        emit queryLogFinished(logQueryResult);
        return;
    }

    if (result.failedEntryCount > 0)
    {
        m_logger.warning(logTag(), QStringLiteral("Skipped %1 faulty log entries while reading log file.").arg(QString::number(result.failedEntryCount)));
    }
    logQueryResult.failedEntryCount = result.failedEntryCount;

    logQueryResult.entries = filterLogEntries(result.request, result.entries);

    emit queryLogFinished(logQueryResult);
}

QList<Entry> LogService::filterLogEntries(const LogQueryRequest &query, const QList<Entry> &entries)
{
    m_logger.verbose(logTag(), QStringLiteral("Filtering log entries..."));

    // Filter entries with query
    QList<Entry> filteredEntries;
    int maxEntryCount = query.maxEntryCount;
    if (maxEntryCount > 0)
    {
        filteredEntries.reserve(maxEntryCount);
    }
    else
    {
        filteredEntries.reserve(entries.size());
    }
    EntrySelectionOrder order = query.entryOrder;
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

    for (const Entry &entry : entries)
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
        case EntrySelectionOrder::Newest:
        {
            m_logger.verbose(logTag(), QStringLiteral("Getting %1 newest entries...").arg(QString::number(maxEntryCount)));
            filteredEntries = filteredEntries.mid(qMax(0, filteredEntries.size() - maxEntryCount));
            m_logger.verbose(logTag(), QStringLiteral("Getting %1 newest entries done!").arg(QString::number(maxEntryCount)));
            break;
        }

        case EntrySelectionOrder::Oldest:
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
