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

using namespace LogLevel;

QString LogLevel::toString(Level level)
{
    switch (level)
    {
    case Level::Info:
        return LogStrings::Info;

    case Level::Warning:
        return LogStrings::Warning;

    case Level::Error:
        return LogStrings::Error;

    case Level::Debug:
        return LogStrings::Debug;

    case Level::Verbose:
        return LogStrings::Verbose;
    }

    // TODO: Assert here? Should get caught during development
    return LogStrings::Unknown;
}

Level LogLevel::fromString(const QString &value)
{
    if (value == LogStrings::Info)
    {
        return Level::Info;
    }

    if (value == LogStrings::Warning)
    {
        return Level::Warning;
    }

    if (value == LogStrings::Error)
    {
        return Level::Error;
    }

    if (value == LogStrings::Debug)
    {
        return Level::Debug;
    }

    if (value == LogStrings::Verbose)
    {
        return Level::Verbose;
    }

    // TODO: Error handling here!
    return Level::Info;
}
