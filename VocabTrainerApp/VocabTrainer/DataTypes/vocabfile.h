#ifndef VOCABFILE_H
#define VOCABFILE_H

#include "error.h"
#include "training.h"

#include <QString>
#include <QList>

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

    struct ReadRequest
    {
        QStringList filePaths;
        Vocab::Direction direction = Vocab::Direction::KeyToValue;
    };

    struct ReadResult
    {
        Error::Code code = Error::Code::Success;
        int failedVocabCount = 0;
        int partiallySucceededVocabCount = 0;
        int failedEntryCount = 0;
        QList<Data> datas;
    };

    struct ReadFileResult
    {
        Error::Code code = Error::Code::Success;
        int failedEntryCount = 0;
        Data data;
    };
}

#endif // VOCABFILE_H
