#include "loglevel.h"

QString logLevelToString(LogLevel level)
{
    switch (level) {
    case LogLevel::Info:
        return QStringLiteral("INFO");

    case LogLevel::Warning:
        return QStringLiteral("WARNING");

    case LogLevel::Error:
        return QStringLiteral("ERROR");

    case LogLevel::Debug:
        return QStringLiteral("DEBUG");

    case LogLevel::Verbose:
        return QStringLiteral("VERBOSE");
    }

    return QStringLiteral("UNKNOWN");
}

LogLevel logLevelFromString(const QString &value)
{
    if (value == QStringLiteral("INFO")) {
        return LogLevel::Info;
    }

    if (value == QStringLiteral("WARNING")) {
        return LogLevel::Warning;
    }

    if (value == QStringLiteral("ERROR")) {
        return LogLevel::Error;
    }

    if (value == QStringLiteral("DEBUG")) {
        return LogLevel::Debug;
    }

    if (value == QStringLiteral("VERBOSE")) {
        return LogLevel::Verbose;
    }

    return LogLevel::Info;
}
