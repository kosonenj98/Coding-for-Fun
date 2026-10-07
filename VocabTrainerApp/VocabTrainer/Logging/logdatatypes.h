#ifndef LOGDATATYPES_H
#define LOGDATATYPES_H

#include "loglevel.h"

#include <QObject>
#include <QDateTime>
#include <QString>

namespace Log
{
    struct Entry
    {
        // Info for context
        Qt::HANDLE threadId;
        QString threadName;
        quint64 sequence;   // LogFileWriter's ordering number
        QString tag;
        QDateTime timestamp;    // Time of logging
        LogLevel level;
        QString message;    // Actual log message
    };

    enum class EntrySelectionOrder
    {
        Oldest,
        Newest
    };

    struct Query
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
}

#endif // LOGDATATYPES_H
