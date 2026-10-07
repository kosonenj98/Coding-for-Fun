#ifndef VOCABFILEDATATYPES_H
#define VOCABFILEDATATYPES_H

#include "errorcode.h"

#include <QList>
#include <QString>

namespace VocabFile
{
    using Entry = QPair<QString,QString>;

    struct Group
    {
        QString title;
        QList<Entry> entries;
    };

    struct Data
    {
        QString title;
        QList<Entry> entries;
        QList<Group> groups;
    };

    enum class ReadMode
    {
        Full,
        Info
    };

    struct Info
    {
        QString filePath;
        QString vocabTitle;
    };

    struct InfoResult
    {
        ErrorCode errorCode = ErrorCode::Success;
        Info info;
    };

    struct ReadResult
    {
        ErrorCode errorCode = ErrorCode::Success;
        int failedEntryCount = 0;
        Data data;
    };
}

#endif // VOCABFILEDATATYPES_H
