#include "vocabtrainer.h"

#include <QThread>

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("VocabTrainer");
        return tag;
    }
}

VocabTrainer::VocabTrainer(Logger &logger, LogFileHandler &handler, QObject *parent)
    : QObject(parent), m_logger(logger), m_logFileHandler(handler)
{
    m_logger.verbose(logTag(), QStringLiteral("Constructing main application..."));
    m_logger.verbose(logTag(), QStringLiteral("Constructing main application done!"));
}

VocabTrainer::~VocabTrainer()
{
    m_logger.verbose(logTag(), QStringLiteral("Destructing main application..."));
    m_logger.verbose(logTag(), QStringLiteral("Destructing main application done!"));
}

void VocabTrainer::initialize()
{
    m_logger.verbose(logTag(), QStringLiteral("Initializing main application..."));
    m_logService = new LogService(m_logger, m_logFileHandler, this);

    connect(m_logService, &LogService::queryLogSucceeded, this, &VocabTrainer::handleLogQuerySucceeded);
    connect(m_logService, &LogService::queryLogSucceededPartially, this, &VocabTrainer::handleLogQuerySucceededPartially);
    connect(m_logService, &LogService::queryLogFailed, this, &VocabTrainer::handleLogQueryFailed);

    m_logger.verbose(logTag(), QStringLiteral("Initializing main application done!"));

    QString threadId = QString::number(reinterpret_cast<quintptr>(QThread::currentThread()->currentThreadId()), 16);
    QString threadName = QThread::currentThread()->objectName();
    m_logger.verbose(logTag(), QStringLiteral("Initializing VocabTrainer done! Running on thread '0x%1' (%2).").arg(threadId, threadName));
}

void VocabTrainer::handleLogQueryRequest(const LogQuery &query)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling log query request..."));
    m_logService->queryLog(query);
}

void VocabTrainer::handleLogQuerySucceeded(const QList<LogEntry> entries)
{
    m_logger.verbose(logTag(), QStringLiteral("Log query succeeded!"));
    m_logger.verbose(logTag(), QStringLiteral("Emitting success signal..."));
    emit logQuerySucceeded(entries);
}

void VocabTrainer::handleLogQuerySucceededPartially(const QList<LogEntry> entries, int failedEntriesCount)
{
    m_logger.verbose(logTag(), QStringLiteral("Log query succeeded partially. Skipped %1 faulty lines in log file.").arg(QString::number(failedEntriesCount)));
    m_logger.verbose(logTag(), QStringLiteral("Emitting partial success signal..."));
    emit logQuerySucceededPartially(entries, failedEntriesCount);
}

void VocabTrainer::handleLogQueryFailed(ErrorCode code)
{
    m_logger.verbose(logTag(), QStringLiteral("Log query failed! ErrorCode: %1").arg(errorCodeToString(code)));
    m_logger.verbose(logTag(), QStringLiteral("Emitting failure signal..."));
    emit logQueryFailed(code);
}

void VocabTrainer::shutdown()
{
    m_logger.verbose(logTag(), QStringLiteral("Shutting down main application..."));
    m_logger.verbose(logTag(),QStringLiteral("Shutting down main application done!"));
}
