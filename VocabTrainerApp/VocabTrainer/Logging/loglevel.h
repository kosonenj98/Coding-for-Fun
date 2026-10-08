#ifndef LOGLEVEL_H
#define LOGLEVEL_H

#include <QString>

namespace LogLevel
{
    enum class Level
    {
        Info,
        Warning,
        Error,
        Debug,
        Verbose
    };

    QString toString(Level level);

    Level fromString(const QString &value);
}

#endif // LOGLEVEL_H
