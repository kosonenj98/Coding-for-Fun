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
    json[LogJsonKeys::Level] = Log::levelToQString(entry.level);
    json[LogJsonKeys::Message] = entry.message;

    const QJsonDocument document(json);
    return QString::fromUtf8(document.toJson(QJsonDocument::Compact));
}

Error::Code LogEntryFormatter::fromJson(const QJsonObject &json, Entry &entry)
{
    const QJsonValue threadIdValue = json.value(LogJsonKeys::ThreadId);
    if (!threadIdValue.isString())
    {
        return Error::Code::InvalidJson;
    }
    bool threadIdOk = false;
    const quint64 threadId = threadIdValue.toString().toULongLong(&threadIdOk, 16);
    if (!threadIdOk)
    {
        return Error::Code::InvalidJson;
    }

    const QJsonValue sequenceValue = json.value(LogJsonKeys::Sequence);
    if (!sequenceValue.isString())
    {
        return Error::Code::InvalidJson;
    }
    bool sequenceOk = false;
    const quint64 sequence = sequenceValue.toString().toULongLong(&sequenceOk);
    if (!sequenceOk)
    {
        return Error::Code::InvalidJson;
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
        return Error::Code::InvalidJson;
    }

    const QDateTime timestamp = QDateTime::fromString(timestampValue.toString(), Qt::ISODate);
    if (!timestamp.isValid())
    {
        return Error::Code::InvalidJson;
    }

    entry.threadId = reinterpret_cast<Qt::HANDLE>(threadId);
    entry.threadName = threadNameValue.toString();
    entry.sequence = sequence;
    entry.tag = tagValue.toString();
    entry.timestamp = timestamp;
    entry.level = Log::levelFromQString(levelValue.toString());
    entry.message = messageValue.toString();

    return Error::Code::Success;
}
