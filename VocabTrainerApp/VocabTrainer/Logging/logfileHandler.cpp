#include "logfileHandler.h"
#include "logentryformatter.h"

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



LogFileHandler::LogFileHandler(SettingsHandler &handler, QObject *parent)
    : QObject{parent}, m_settingsHandler(handler), m_logFile(handler.getLogFilePath())
{
    // Cannot log before initializing is finished!
}

void LogFileHandler::initialize()
{
    if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        // TODO: Handle this error!
    }
    m_flushTimer = new QTimer(this);
    connect(m_flushTimer, &QTimer::timeout, this, &LogFileHandler::flush);
    m_flushTimer->start(FlushTimerIntervalMs);
}

void LogFileHandler::writeEntry(const LogEntry &entry)
{
    if (!m_logFile.isOpen())
    {
        return;
    }

    if (!isLoggingEnabled(entry))
    {
        return;
    }

    // Produce log file entry and write it to log
    LogEntry fileEntry = entry; // Create a local copy so the sequence can be assigned for file output
    fileEntry.sequence = m_sequence++;
    const QByteArray line = LogEntryFormatter::logFileFormat(fileEntry).toUtf8() + '\n';
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
            // TODO: Emit error here
            return;
        }
    }

    m_logFile.setFileName(newFilePath);

    if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        // TODO: Emit error here
        return;
    }

    info(QStringLiteral("Log file location changed successfully. Previous log: '%1'").arg(oldFilePath));

    verbose(QStringLiteral("Setting new log file path done!"));
}

void LogFileHandler::readAllLogEntries(const LogQuery &query)
{
    debug(QStringLiteral("Reading log entries..."));
    QList<LogEntry> entries;

    QString filePath = query.filePath;
    QFile file(filePath);

    verbose(QStringLiteral("Opening file '%1'...").arg(filePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        error(QStringLiteral("Failed to open '%1': '%2' (FileError '%3')").arg(filePath, file.errorString(), QString::number(file.error())));
        emit readAllLogEntriesFailed(ErrorCode::FileOpenFailed);
        return;
    }
    verbose(QStringLiteral("Opening file '%1' succeeded!").arg(filePath));

    debug(QStringLiteral("Reading log entries..."));
    int failedEntriesCount = 0;
    while (!file.atEnd())
    {
        const QByteArray line = file.readLine().trimmed();

        if (line.isEmpty())
        {
            continue;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(line, &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject())
        {
            error(QStringLiteral("Parsing log file failed: '%1' (%2). Line: '%3'").arg(parseError.errorString(), QString::number(parseError.error), QString::fromUtf8(line)));
            debug(QStringLiteral("Skipping the problematic line..."));
            failedEntriesCount++;
            continue;
        }

        LogEntry entry;
        const ErrorCode errorCode = LogEntryFormatter::fromJson(document.object(), entry);
        if (errorCode != ErrorCode::Success)
        {
            error(QStringLiteral("Parsing log entry failed! Line: '%1' (ErrorCode '%2')").arg(QString::fromUtf8(line), QString::number(static_cast<int>(errorCode))));
            failedEntriesCount++;
            continue;
        }

        entries.append(entry);
    }

    file.close();

    debug(QStringLiteral("Reading log entries done!"));
    if (failedEntriesCount == 0)
    {
        verbose(QStringLiteral("Emitting log entry reading success signal..."));
        emit readAllLogEntriesSucceeded(query, entries);
    }
    else
    {
        warning(QStringLiteral("Skipped %1 faulty entries in log file '%2'.").arg(QString::number(failedEntriesCount), filePath));
        verbose(QStringLiteral("Emitting log entry reading partial success signal..."));
        emit readAllLogEntriesSucceededPartially(query, entries, failedEntriesCount);
    }
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

    // Respect LogFileHandler's logging queue
    QMetaObject::invokeMethod(this, &LogFileHandler::writeEntry, Qt::QueuedConnection, entry);
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

    // TODO: Assert here?
    return true;
}
