#include "logfilewriter.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QFileInfo>
#include <QThread>

namespace
{
    const QString LOG_TAG = QStringLiteral("LogFileWriter");
}

LogFileWriter::LogFileWriter(const QString &filePath, QObject *parent)
    : QObject{parent}, m_logFile(filePath)
{
}

void LogFileWriter::initialize()
{
    if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        m_logFileAvailable = false;
    }
    m_logFileAvailable = true;

    m_flushTimer = new QTimer(this);
    connect(m_flushTimer, &QTimer::timeout, this, &LogFileWriter::flush);
    m_flushTimer->start(FlushTimerIntervalMs);
}

void LogFileWriter::writeEntry(const LogEntry &entry)
{
    if (!m_logFileAvailable)
    {
        return;
    }

    QJsonObject json;
    json["threadId"] = QString::number(reinterpret_cast<quintptr>(entry.threadId));
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

void LogFileWriter::flush()
{
    if (!m_logFile.isOpen() || m_entriesSinceFlush == 0)
    {
        return;
    }

    m_logFile.flush();
    m_entriesSinceFlush = 0;
}

void LogFileWriter::setFilePath(const QString &newFilePath)
{
    QString oldFilePath = m_logFile.fileName();
    if (oldFilePath == newFilePath)
    {
        return;
    }

    logInternally(LogLevel::Info, QStringLiteral("Changing log file location to '%1'...").arg(newFilePath));

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

    logInternally(LogLevel::Info, QStringLiteral("Log file location changed successfully. Previous log: '%1'").arg(oldFilePath));
}

void LogFileWriter::logInternally(LogLevel level, const QString &message)
{
    LogEntry entry;

    entry.threadId = QThread::currentThreadId();
    entry.threadName = QThread::currentThread()->objectName();
    entry.sequence = 0; // writeEntry updates
    entry.tag = LOG_TAG;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = level;
    entry.message = message;

    writeEntry(entry);
}

QString LogFileWriter::logLevelToString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::Info:
        return QStringLiteral("INFO");

    case LogLevel::Warning:
        return QStringLiteral("WARNING");

    case LogLevel::Error:
        return QStringLiteral("ERROR");

    case LogLevel::Debug:
        return QStringLiteral("DEBUG");

    case LogLevel::Verbose:
        return QStringLiteral("VERBOSE");
    }

    return QStringLiteral("UNKNOWN");
}
