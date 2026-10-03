#include "logger.h"

#include <QThread>

Logger::Logger(QObject *parent)
    : QObject{parent}
{}

void Logger::log(LogLevel level, const QString &tag, const QString &message)
{
    QThread *thread = QThread::currentThread();
    QString threadName = thread->objectName();

    LogEntry entry;
    entry.threadId = QThread::currentThreadId();
    entry.threadName = threadName.isEmpty() ? QStringLiteral("Unnamed") : threadName;
    entry.sequence = 0; // LogFileWriter updates this
    entry.tag = tag;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = level;
    entry.message = message;

    emit logEntryCreated(entry);
}

void Logger::info(const QString &tag, const QString &message)
{
    log(LogLevel::Info, tag, message);
}

void Logger::warning(const QString &tag, const QString &message)
{
    log(LogLevel::Warning, tag, message);
}

void Logger::error(const QString &tag, const QString &message)
{
    log(LogLevel::Error, tag, message);
}

void Logger::debug(const QString &tag, const QString &message)
{
    log(LogLevel::Debug, tag, message);
}

void Logger::verbose(const QString &tag, const QString &message)
{
    log(LogLevel::Verbose, tag, message);
}
