#include "loglevel.h"

namespace LogStrings
{
    inline const QString Info = QStringLiteral("INFO");
    inline const QString Warning = QStringLiteral("WARNING");
    inline const QString Error = QStringLiteral("ERROR");
    inline const QString Debug = QStringLiteral("DEBUG");
    inline const QString Verbose = QStringLiteral("VERBOSE");
    inline const QString Unknown = QStringLiteral("UNKNOWN");
}

QString logLevelToString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::Info:
        return LogStrings::Info;

    case LogLevel::Warning:
        return LogStrings::Warning;

    case LogLevel::Error:
        return LogStrings::Error;

    case LogLevel::Debug:
        return LogStrings::Debug;

    case LogLevel::Verbose:
        return LogStrings::Verbose;
    }

    // TODO: Assert here? Should get caught during development
    return LogStrings::Unknown;
}

LogLevel logLevelFromString(const QString &value)
{
    if (value == LogStrings::Info)
    {
        return LogLevel::Info;
    }

    if (value == LogStrings::Warning)
    {
        return LogLevel::Warning;
    }

    if (value == LogStrings::Error)
    {
        return LogLevel::Error;
    }

    if (value == LogStrings::Debug)
    {
        return LogLevel::Debug;
    }

    if (value ==LogStrings::Verbose)
    {
        return LogLevel::Verbose;
    }

    // TODO: Error handling here!
    return LogLevel::Info;
}
