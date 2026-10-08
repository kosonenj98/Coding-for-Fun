#ifndef LOGGER_H
#define LOGGER_H

#include "DataTypes/datatypes.h"
#include "loglevel.h"

#include <QObject>
#include <QDateTime>

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(QObject *parent = nullptr);

    void log(LogLevel::Level level, const QString &tag, const QString &message);
    void info(const QString &tag, const QString &message);
    void warning(const QString &tag, const QString &message);
    void error(const QString &tag, const QString &message);
    void debug(const QString &tag, const QString &message);
    void verbose(const QString &tag, const QString &message);

signals:
    void logEntryCreated(const Log::Entry &entry);
};

#endif // LOGGER_H
