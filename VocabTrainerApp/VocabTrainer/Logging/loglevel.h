#ifndef LOGLEVEL_H
#define LOGLEVEL_H

#include <QString>

enum class LogLevel
{
    Info,
    Warning,
    Error,
    Debug,
    Verbose
};

QString logLevelToString(LogLevel level);

LogLevel logLevelFromString(const QString &value);

#endif // LOGLEVEL_H
