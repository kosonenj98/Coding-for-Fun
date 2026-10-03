#include "logfileHandler.h"
#include "logger.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QFileInfo>
#include <QThread>

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("LogFileHandler");
        return tag;
    }
}

LogFileHandler::LogFileHandler(const QString &filePath, QObject *parent)
    : QObject{parent}, m_logFile(filePath)
{
}

void LogFileHandler::initialize()
{
    if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        m_logFileAvailable = false;
    }
    m_logFileAvailable = true;

    m_flushTimer = new QTimer(this);
    connect(m_flushTimer, &QTimer::timeout, this, &LogFileHandler::flush);
    m_flushTimer->start(FlushTimerIntervalMs);
}

void LogFileHandler::writeEntry(const LogEntry &entry)
{
    if (!m_logFileAvailable)
    {
        return;
    }

    QJsonObject json;
    json["threadId"] = QString::number(reinterpret_cast<quintptr>(entry.threadId), 16);
    json["threadName"] = entry.threadName;
    json["sequence"] = static_cast<qint64>(m_sequence++);
    json["tag"] = entry.tag;
    json["timestamp"] = entry.timestamp.toString(Qt::ISODateWithMs);
    json["level"] = logLevelToString(entry.level);
    json["message"] = entry.message;

    const QJsonDocument document(json);
    const QByteArray line = document.toJson(QJsonDocument::Compact) + '\n';

    m_logFile.write(line);
    m_entriesSinceFlush++;

    if (entry.level == LogLevel::Error || m_entriesSinceFlush >= FlushInterval)
    {
        flush();
    }
}

void LogFileHandler::flush()
{
    if (!m_logFile.isOpen() || m_entriesSinceFlush == 0)
    {
        return;
    }

    m_logFile.flush();
    m_entriesSinceFlush = 0;
}

void LogFileHandler::setFilePath(const QString &newFilePath)
{
    verbose(QStringLiteral("Setting new log file path..."));

    QString oldFilePath = m_logFile.fileName();
    if (oldFilePath == newFilePath)
    {
        return;
    }
    verbose(QStringLiteral("Previous log file: '%1'").arg(oldFilePath));
    verbose(QStringLiteral("New file: '%2'").arg(newFilePath));

    info(QStringLiteral("Changing log file location to '%1'...").arg(newFilePath));

    flush();

    if (m_logFile.isOpen())
    {
        m_logFile.close();
    }

    QFileInfo fileInfo(newFilePath);
    if (!fileInfo.absoluteDir().exists())
    {
        if (!fileInfo.absoluteDir().mkpath(QStringLiteral(".")))
        {
            m_logFileAvailable = false;
            return;
        }
    }

    m_logFile.setFileName(newFilePath);

    if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        m_logFileAvailable = false;
        return;
    }

    m_logFileAvailable = true;

    info(QStringLiteral("Log file location changed successfully. Previous log: '%1'").arg(oldFilePath));

    verbose(QStringLiteral("Setting new log file path done!"));
}

void LogFileHandler::readEntries(const LogQuery &query)
{
    verbose(QStringLiteral("Reading log entries..."));
    QList<LogEntry> entries;

    QFile file(query.filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // TODO: Emit file error
        emit readEntriesFinished(query, entries);
        return;
    }

    while (!file.atEnd())
    {
        const QByteArray line = file.readLine().trimmed();

        if (line.isEmpty())
        {
            continue;
        }

        QJsonParseError parseError;
        const QJsonDocument document =
            QJsonDocument::fromJson(line, &parseError);

        if (parseError.error != QJsonParseError::NoError || !document.isObject())
        {
            // TODO: Handle malformed log line
            continue;
        }

        const QJsonObject json = document.object();

        LogEntry entry;
        entry.threadId = reinterpret_cast<Qt::HANDLE>(json.value(QStringLiteral("threadId")).toString().toULongLong(nullptr, 16));
        entry.threadName = json.value(QStringLiteral("threadName")).toString();
        entry.sequence = json.value(QStringLiteral("sequence")).toVariant().toULongLong();
        entry.tag = json.value(QStringLiteral("tag")).toString();
        entry.timestamp = QDateTime::fromString(json.value(QStringLiteral("timestamp")).toString(), Qt::ISODate);
        entry.level = logLevelFromString(json.value(QStringLiteral("level")).toString());
        entry.message = json.value(QStringLiteral("message")).toString();

        entries.append(entry);
    }

    file.close();

    verbose(QStringLiteral("Reading log entries done!"));
    emit readEntriesFinished(query, entries);
}

void LogFileHandler::shutdown()
{
    verbose(QStringLiteral("Shutting down..."));
    flush();

    if (m_logFile.isOpen())
    {
        m_logFile.close();
    }

    if (m_flushTimer->isActive())
    {
        m_flushTimer->stop();
    }
}

void LogFileHandler::logInternally(LogLevel level, const QString &message)
{
    LogEntry entry;
    entry.threadId = QThread::currentThreadId();
    entry.threadName = QThread::currentThread()->objectName();
    entry.sequence = 0; // writeEntry updates
    entry.tag = logTag();
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = level;
    entry.message = message;

    writeEntry(entry);
}

void LogFileHandler::info(const QString &message)
{
    logInternally(LogLevel::Info, message);
}

void LogFileHandler::warning(const QString &message)
{
    logInternally(LogLevel::Warning, message);
}

void LogFileHandler::error(const QString &message)
{
    logInternally(LogLevel::Error, message);
}

void LogFileHandler::debug(const QString &message)
{
    logInternally(LogLevel::Debug, message);
}

void LogFileHandler::verbose(const QString &message)
{
    logInternally(LogLevel::Verbose, message);
}
