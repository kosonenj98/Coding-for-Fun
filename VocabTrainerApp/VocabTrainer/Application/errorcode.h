#ifndef ERRORCODE_H
#define ERRORCODE_H

#include <QString>

enum class ErrorCode
{
    Success,
    FileOpenFailed,
    FileReadFailed,
    FileWriteFailed,
    InvalidJson,
    InvalidSettings,
    VocabFileNoEntries,
    NoSucceededVocabs,
    NoVocabFilesGiven,
    Unknown
};

QString errorCodeToString(ErrorCode code);

#endif // ERRORCODE_H
