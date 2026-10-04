#ifndef LOGDATATYPES_H
#define LOGDATATYPES_H

#include "loglevel.h"

#include <QObject>
#include <QDateTime>
#include <QString>

struct LogEntry
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

enum class LogEntrySelectionOrder
{
    Oldest,
    Newest
};

struct LogQuery
{
    QString filePath;

    int maxEntryCount = 0;
    LogEntrySelectionOrder entryOrder = LogEntrySelectionOrder::Newest;

    QString searchPattern;
    bool useRegularExpression = false;
    bool searchEntireEntry = false;

    QDateTime from;
    QDateTime to;
};

#endif // LOGDATATYPES_H
