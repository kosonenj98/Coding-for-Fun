#include "logfilehandler.h"
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

using namespace Log;

LogFileHandler::LogFileHandler(SettingsHandler &handler, QObject *parent)
    : QObject{parent}, m_settingsHandler(handler), m_logFile(handler.settings().logFilePath)
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

void LogFileHandler::writeEntry(const Entry &entry)
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
    Entry fileEntry = entry; // Create a local copy so the sequence can be assigned for file output
    fileEntry.sequence = m_sequence++;
    const QByteArray line = LogEntryFormatter::logFileFormat(fileEntry).toUtf8() + '\n';
    m_logFile.write(line);
    m_entriesSinceFlush++;

    // Flush error messaged immediately. Else flush if flush interval is exceeded
    if (entry.level == Log::Level::Error || m_entriesSinceFlush >= FlushInterval)
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

void LogFileHandler::handleGetAllLogEntriesRequest(const GetAllEntriesRequest &request)
{
    Log::GetAllEntriesResult result;
    result.query = request.query;

    const QString &filePath = request.query.filePath;
    verbose(QStringLiteral("Opening file '%1'...").arg(filePath));
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        error(QStringLiteral("Failed to open '%1': '%2' (FileError '%3')").arg(filePath, file.errorString(), QString::number(file.error())));
        result.code = Error::Code::FileOpenFailed;
        emit getAllLogEntriesFinished(result);
        return;
    }
    verbose(QStringLiteral("Opening file '%1' succeeded!").arg(filePath));

    debug(QStringLiteral("Reading log entries..."));
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
            result.failedEntryCount++;
            continue;
        }

        Entry entry;
        const Error::Code code = LogEntryFormatter::fromJson(document.object(), entry);
        if (code != Error::Code::Success)
        {
            error(QStringLiteral("Parsing log entry failed! Line: '%1' (ErrorCode '%2'). Skipping...").arg(QString::fromUtf8(line), QString::number(static_cast<int>(code))));
            result.failedEntryCount++;
            continue;
        }

        result.entries.append(entry);
    }

    file.close();

    if (result.failedEntryCount > 0)
    {
        warning(QStringLiteral("Skipped %1 faulty log entries in log file '%2'").arg(QString::number(result.failedEntryCount), filePath));
    }

    emit getAllLogEntriesFinished(result);
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

void LogFileHandler::logInternally(Log::Level level, const QString &message)
{
    Entry entry;
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
    logInternally(Log::Level::Info, message);
}

void LogFileHandler::warning(const QString &message)
{
    logInternally(Log::Level::Warning, message);
}

void LogFileHandler::error(const QString &message)
{
    logInternally(Log::Level::Error, message);
}

void LogFileHandler::debug(const QString &message)
{
    logInternally(Log::Level::Debug, message);
}

void LogFileHandler::verbose(const QString &message)
{
    logInternally(Log::Level::Verbose, message);
}

bool LogFileHandler::isLoggingEnabled(const Entry &entry)
{
    const Settings::Settings &settings = m_settingsHandler.settings();
    if (!settings.logEnabled)
    {
        // Logging is disabled from settings
        return false;
    }

    // Check if entry's log level is enabled
    switch (entry.level)
    {
    case Log::Level::Info:
        return settings.logInfoEnabled;

    case Log::Level::Warning:
        return settings.logWarningEnabled;

    case Log::Level::Error:
        return settings.logErrorEnabled;

    case Log::Level::Debug:
        return settings.logDebugEnabled;

    case Log::Level::Verbose:
        return settings.logVerboseEnabled;
    }

    // TODO: Assert here?
    return true;
}
