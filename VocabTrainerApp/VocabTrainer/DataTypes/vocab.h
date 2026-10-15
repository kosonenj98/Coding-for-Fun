#ifndef VOCAB_H
#define VOCAB_H

#include "error.h"

#include <QMap>
#include <QString>

namespace Vocab
{
    using Entries = QMap<QString,QSet<QString>>;

    using GroupEntries = QMap<QString,Entries>;

    struct GroupContentInfo
    {
        QString title;
        int entryCount;
    };

    struct ContentInfo
    {
        QString title;
        int entryCount;
        QList<GroupContentInfo> groupInfos;
        int totalEntryCount;
    };

    struct Data
    {
        QString title;
        Entries entries;
        GroupEntries groupEntries;
    };

    enum class Direction
    {
        KeyToValue,
        ValueToKey
    };

    struct LoadRequest
    {
        QStringList filePaths;
        Direction direction;
    };

    struct LoadResult
    {
        Error::Code code = Error::Code::Success;
        int failedVocabCount = 0;
        int partiallySucceededVocabCount = 0;
        int failedEntryCount = 0;
        QList<ContentInfo> contentInfos;
    };

    struct ClearRequest
    {
    };

    struct ClearResult
    {
        Error::Code code = Error::Code::Success;
    };

    struct Filter
    {
        QString title;
        bool includeEntriesWithoutGroup;
        QStringList groupTitles;
    };

    struct GetRequest
    {
        QList<Filter> filters;
    };

    struct GetResult
    {
        Error::Code code = Error::Code::Success;
        int totalEntryCount;
        QList<Vocab::Data> datas;
    };


}

#endif // VOCAB_H
