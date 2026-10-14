#ifndef LOGENTRYFORMATTER_H
#define LOGENTRYFORMATTER_H

#include "DataTypes/error.h"
#include "DataTypes/log.h"

#include <QString>
#include <QJsonDocument>

namespace LogJsonKeys
{
    inline const QString ThreadId = QStringLiteral("threadId");
    inline const QString ThreadName = QStringLiteral("threadName");
    inline const QString Sequence = QStringLiteral("sequence");
    inline const QString Tag = QStringLiteral("tag");
    inline const QString Timestamp = QStringLiteral("timestamp");
    inline const QString Level = QStringLiteral("level");
    inline const QString Message = QStringLiteral("message");
}

namespace LogEntryFormatter
{
    QString logFileFormat(const Log::Entry& entry);

    Error::Code fromJson(const QJsonObject &json, Log::Entry &entry);
}

#endif // LOGENTRYFORMATTER_H
