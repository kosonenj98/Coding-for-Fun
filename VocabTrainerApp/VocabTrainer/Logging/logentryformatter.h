#ifndef LOGENTRYFORMATTER_H
#define LOGENTRYFORMATTER_H

#include "logdatatypes.h"
#include "../Application/errorcode.h"

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

ErrorCode fromJson(const QJsonObject &json, Log::Entry &entry);
}

#endif // LOGENTRYFORMATTER_H
