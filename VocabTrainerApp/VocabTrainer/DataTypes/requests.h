#ifndef REQUESTS_H
#define REQUESTS_H

#include "datatypes.h"

struct LoadVocabRequest
{
    VocabFile::Info info;
    VocabFile::ReadMode mode = VocabFile::ReadMode::Data;
};

struct LoadVocabsRequest
{
    QList<VocabFile::Info> infos;
    VocabFile::ReadMode mode = VocabFile::ReadMode::Data;;
};

struct LogQueryRequest
{
    QString filePath;

    int maxEntryCount = 0;
    Log::EntrySelectionOrder entryOrder = Log::EntrySelectionOrder::Newest;

    QString searchPattern;
    bool useRegularExpression = false;
    bool searchEntireEntry = false;

    QDateTime from;
    QDateTime to;
};

struct GetAllLogEntriesRequest
{
    LogQueryRequest request;
};

struct ApplyNewSettingsRequest
{
    Settings settings;
};

struct ReadVocabFileRequest
{
    QString filePath;
    VocabFile::ReadMode mode;
};

struct ReadVocabFilesRequest
{
    QStringList filePaths;
    VocabFile::ReadMode mode;
};

#endif // REQUESTS_H
