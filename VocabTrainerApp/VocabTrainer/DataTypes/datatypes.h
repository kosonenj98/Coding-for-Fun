#ifndef DATATYPES_H
#define DATATYPES_H

#include "Logging/loglevel.h"

#include <QString>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>

namespace VocabFile
{
    enum class ReadMode
    {
        Info,
        Data,
    };

    struct Info
    {
        QString filePath;
        QString vocabTitle;
    };

    using Entry = QPair<QString,QString>;

    struct Group
    {
        QString title;
        QList<Entry> entries;
    };

    struct Data
    {
        Info info;
        QList<Entry> entries;
        QList<Group> groups;
    };
}

namespace Vocab
{
    using Entries = QMap<QString,QSet<QString>>;

    using GroupEntries = QMap<QString,Entries>;

    struct Data
    {
        QString title;
        Entries entries;
        GroupEntries groupEntries;
    };
}

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
        LogLevel::Level level;
        QString message;    // Actual log message
    };

    enum class EntrySelectionOrder
    {
        Oldest,
        Newest
    };
}

struct Settings
{
    Settings()
    {
        logEnabled = true;
        logInfoEnabled = true;
        logWarningEnabled = true;
        logErrorEnabled = true;
        logDebugEnabled = true;
        logVerboseEnabled = true;

        const QString logDirectory = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        if (!QDir().mkpath(logDirectory))
        {
            // TODO: Handle error!
        }
        logFilePath = QDir(logDirectory).filePath(QStringLiteral("vocabtrainer.log"));
    }

    bool logEnabled;
    bool logInfoEnabled;
    bool logWarningEnabled;
    bool logErrorEnabled;
    bool logDebugEnabled;
    bool logVerboseEnabled;

    QString logFilePath;
};

#endif // DATATYPES_H
