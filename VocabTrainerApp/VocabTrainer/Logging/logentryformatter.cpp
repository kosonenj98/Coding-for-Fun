#include "logentryformatter.h"

#include <QJsonDocument>
#include <QJsonObject>

using namespace Log;

QString LogEntryFormatter::logFileFormat(const Entry &entry)
{
    // Formulate log entry for log file
    QJsonObject json;
    json[LogJsonKeys::ThreadId] = QString::number(reinterpret_cast<quintptr>(entry.threadId), 16);
    json[LogJsonKeys::ThreadName] = entry.threadName;
    json[LogJsonKeys::Sequence] = QString::number(entry.sequence);
    json[LogJsonKeys::Tag] = entry.tag;
    json[LogJsonKeys::Timestamp] = entry.timestamp.toString(Qt::ISODateWithMs);
    json[LogJsonKeys::Level] = logLevelToString(entry.level);
    json[LogJsonKeys::Message] = entry.message;

    const QJsonDocument document(json);
    return QString::fromUtf8(document.toJson(QJsonDocument::Compact));
}

ErrorCode LogEntryFormatter::fromJson(const QJsonObject &json, Entry &entry)
{
    const QJsonValue threadIdValue = json.value(LogJsonKeys::ThreadId);
    if (!threadIdValue.isString())
    {
        return ErrorCode::InvalidJson;
    }
    bool threadIdOk = false;
    const quint64 threadId = threadIdValue.toString().toULongLong(&threadIdOk, 16);
    if (!threadIdOk)
    {
        return ErrorCode::InvalidJson;
    }

    const QJsonValue sequenceValue = json.value(LogJsonKeys::Sequence);
    if (!sequenceValue.isString())
    {
        return ErrorCode::InvalidJson;
    }
    bool sequenceOk = false;
    const quint64 sequence = sequenceValue.toString().toULongLong(&sequenceOk);
    if (!sequenceOk)
    {
        return ErrorCode::InvalidJson;
    }

    const QJsonValue threadNameValue = json.value(LogJsonKeys::ThreadName);
    const QJsonValue tagValue = json.value(LogJsonKeys::Tag);
    const QJsonValue timestampValue = json.value(LogJsonKeys::Timestamp);
    const QJsonValue levelValue = json.value(LogJsonKeys::Level);
    const QJsonValue messageValue = json.value(LogJsonKeys::Message);
    if (!threadNameValue.isString() ||
        !tagValue.isString() ||
        !timestampValue.isString() ||
        !levelValue.isString() ||
        !messageValue.isString())
    {
        return ErrorCode::InvalidJson;
    }

    const QDateTime timestamp = QDateTime::fromString(timestampValue.toString(), Qt::ISODate);
    if (!timestamp.isValid())
    {
        return ErrorCode::InvalidJson;
    }

    entry.threadId = reinterpret_cast<Qt::HANDLE>(threadId);
    entry.threadName = threadNameValue.toString();
    entry.sequence = sequence;
    entry.tag = tagValue.toString();
    entry.timestamp = timestamp;
    entry.level = logLevelFromString(levelValue.toString());
    entry.message = messageValue.toString();

    return ErrorCode::Success;
}
