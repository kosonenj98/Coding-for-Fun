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

LogFileHandler::LogFileHandler(SettingsHandler &handler, QObject *parent)
    : QObject{parent}, m_settingsHandler(handler), m_logFile(handler.getLogFilePath())
{
    // Cannot log before initializing is finished!
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

    if (!isLoggingEnabled(entry))
    {
        return;
    }

    // Produce log file entry and write it to log
    QJsonObject json;
    json[LogJsonKeys::ThreadId] = QString::number(reinterpret_cast<quintptr>(entry.threadId), 16);
    json[LogJsonKeys::ThreadName] = entry.threadName;
    json[LogJsonKeys::Sequence] = static_cast<qint64>(m_sequence++);
    json[LogJsonKeys::Tag] = entry.tag;
    json[LogJsonKeys::Timestamp] = entry.timestamp.toString(Qt::ISODateWithMs);
    json[LogJsonKeys::Level] = logLevelToString(entry.level);
    json[LogJsonKeys::Message] = entry.message;

    const QJsonDocument document(json);
    const QByteArray line = document.toJson(QJsonDocument::Compact) + '\n';

    m_logFile.write(line);
    m_entriesSinceFlush++;

    // Flush error messaged immediately. Else flush if flush interval is exceeded
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
        entry.threadId = reinterpret_cast<Qt::HANDLE>(json.value(LogJsonKeys::ThreadId).toString().toULongLong(nullptr, 16));
        entry.threadName = json.value(LogJsonKeys::ThreadName).toString();
        entry.sequence = json.value(LogJsonKeys::Sequence).toVariant().toULongLong();
        entry.tag = json.value(LogJsonKeys::Tag).toString();
        entry.timestamp = QDateTime::fromString(json.value(LogJsonKeys::Timestamp).toString(), Qt::ISODate);
        entry.level = logLevelFromString(json.value(LogJsonKeys::Level).toString());
        entry.message = json.value(LogJsonKeys::Message).toString();

        entries.append(entry);
    }

    file.close();

    verbose(QStringLiteral("Reading log entries done!"));
    emit readEntriesFinished(query, entries);
}

void LogFileHandler::shutdown()
{
    verbose(QStringLiteral("Shutting down... Goodbye!"));
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

bool LogFileHandler::isLoggingEnabled(const LogEntry &entry)
{
    if (!m_settingsHandler.getLogEnabled())
    {
        // Logging is disabled from settings
        return false;
    }

    // Check if entry's log level is enabled
    switch (entry.level)
    {
    case LogLevel::Info:
        return m_settingsHandler.getLogInfoEnabled();

    case LogLevel::Warning:
        return m_settingsHandler.getLogWarningEnabled();

    case LogLevel::Error:
        return m_settingsHandler.getLogErrorEnabled();

    case LogLevel::Debug:
        return m_settingsHandler.getLogDebugEnabled();

    case LogLevel::Verbose:
        return m_settingsHandler.getLogVerboseEnabled();
    }

    // TODO: Asset here?
    return true;
}
