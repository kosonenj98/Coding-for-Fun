#ifndef LOG_H
#define LOG_H

#include "error.h"

#include <QString>
#include <QDateTime>

namespace Log
{
    enum class Level
    {
        Info,
        Warning,
        Error,
        Debug,
        Verbose
    };

    QString levelToQString(Level level);
    Level levelFromQString(const QString &value);

    struct Entry
    {
        // Info for context
        Qt::HANDLE threadId;
        QString threadName;
        quint64 sequence; // LogFileWriter's ordering number
        QString tag;
        QDateTime timestamp; // Time of logging
        Level level;
        QString message; // Actual log message
    };

    enum class EntrySelectionOrder
    {
        Oldest,
        Newest
    };

    struct QueryRequest
    {
        QString filePath;

        int maxEntryCount = 0;
        EntrySelectionOrder entryOrder = EntrySelectionOrder::Newest;

        QString searchPattern;
        bool useRegularExpression = false;
        bool searchEntireEntry = false;

        QDateTime from;
        QDateTime to;
    };

    struct QueryResult
    {
        Error::Code code = Error::Code::Success;
        int failedEntryCount = 0;
        QList<Entry> entries;
    };

    struct GetAllEntriesRequest
    {
        QueryRequest query;
    };

    struct GetAllEntriesResult
    {
        Error::Code code = Error::Code::Success;
        int failedEntryCount = 0;
        QueryRequest query;
        QList<Entry> entries;
    };
}

#endif // LOG_H
