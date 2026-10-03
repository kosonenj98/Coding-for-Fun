#ifndef ERRORCODE_H
#define ERRORCODE_H

#include <QString>

enum class ErrorCode
{
    Unknown,
    FileOpenFailed,
    FileReadFailed,
    FileWriteFailed,
    InvalidJson,
    InvalidSettings
};

QString errorCodeToString(ErrorCode code);

#endif // ERRORCODE_H
