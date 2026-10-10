#ifndef RESULTS_H
#define RESULTS_H

#include "datatypes.h"
#include "requests.h"
#include "Application/errorcode.h"

struct LoadVocabInfoResult
{
    ErrorCode code = ErrorCode::Success;
    VocabFile::Info info;
};

struct LoadVocabInfosResult
{
    ErrorCode code = ErrorCode::Success;
    QList<VocabFile::Info> infos;
};

struct LoadVocabDataResult
{
    ErrorCode code = ErrorCode::Success;
    int failedEntryCount = 0;
};

struct LoadVocabDatasResult
{
    ErrorCode code = ErrorCode::Success;
    int failedVocabCount = 0;
    int partiallySucceededVocabCount = 0;
    int failedEntryCount = 0;
};

struct GenerateVocabDataResult
{
    ErrorCode code = ErrorCode::Success;
    int failedGroupCount = 0;
    int partiallySucceededGroupCount = 0;
    int failedEntryCount = 0;
    Vocab::Data data;
};

struct GetAllLogEntriesResult
{
    ErrorCode code = ErrorCode::Success;
    int failedEntryCount = 0;
    LogQueryRequest request;
    QList<Log::Entry> entries;
};

struct ApplyNewSettingsResult
{
    ErrorCode code = ErrorCode::Success;
    Settings settings;
};

struct ReadVocabFileInfoResult
{
    ErrorCode code = ErrorCode::Success;
    VocabFile::Info info;
};

struct ReadVocabFileDataResult
{
    ErrorCode code = ErrorCode::Success;
    int failedEntryCount = 0;
    VocabFile::Data data;
};

struct ReadVocabFileInfosResult
{
    ErrorCode code = ErrorCode::Success;
    int failedVocabCount = 0;
    QList<VocabFile::Info> infos;
};

struct ReadVocabFileDatasResult
{
    ErrorCode code = ErrorCode::Success;
    int failedVocabCount = 0;
    int partiallySucceededVocabCount = 0;
    int failedEntryCount = 0;
    QList<VocabFile::Data> datas;
};

struct LogQueryResult
{
    ErrorCode code = ErrorCode::Success;
    int failedEntryCount = 0;
    QList<Log::Entry> entries;
};

struct ReadAllResult
{
    ErrorCode code = ErrorCode::Success;
    int failedEntryCount = 0;
    QList<Log::Entry> entries;
};

struct GenerateVocabEntriesResult
{
    ErrorCode code = ErrorCode::Success;
    int failedEntryCount = 0;
    Vocab::Entries entries;
};

struct GenerateVocabEntryVariantsResult
{
    ErrorCode code = ErrorCode::Success;
    Vocab::Entries entries;
};

#endif // RESULTS_H
