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

    // Initialize services and handlers
    initializeLogService();
    initializeVocabFileHandler();
    initializeDataService();

    m_logger.verbose(logTag(), QStringLiteral("Initializing main application done!"));

    QString threadId = QString::number(reinterpret_cast<quintptr>(QThread::currentThread()->currentThreadId()), 16);
    QString threadName = QThread::currentThread()->objectName();
    m_logger.verbose(logTag(), QStringLiteral("Initializing VocabTrainer done! Running on thread '0x%1' (%2).").arg(threadId, threadName));
}

void VocabTrainer::handleLogQueryRequest(const Log::Query &query)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling log query request..."));
    m_logService->queryLog(query);
}

void VocabTrainer::handleLogQuerySucceeded(const QList<Log::Entry> entries)
{
    m_logger.verbose(logTag(), QStringLiteral("Log query succeeded!"));
    m_logger.verbose(logTag(), QStringLiteral("Emitting success signal..."));
    emit logQuerySucceeded(entries);
}

void VocabTrainer::handleLogQuerySucceededPartially(const QList<Log::Entry> entries, int failedEntryCount)
{
    m_logger.verbose(logTag(), QStringLiteral("Log query succeeded partially. Skipped %1 faulty lines in log file.").arg(QString::number(failedEntryCount)));
    m_logger.verbose(logTag(), QStringLiteral("Emitting partial success signal..."));
    emit logQuerySucceededPartially(entries, failedEntryCount);
}

void VocabTrainer::handleLogQueryFailed(const ErrorCode code)
{
    m_logger.verbose(logTag(), QStringLiteral("Log query failed! ErrorCode: %1").arg(errorCodeToString(code)));
    m_logger.verbose(logTag(), QStringLiteral("Emitting failure signal..."));
    emit logQueryFailed(code);
}

void VocabTrainer::handleVocabLoadRequest(const QString &filePath)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling vocab load request..."));
    m_dataService->loadVocabData(filePath);
}

void VocabTrainer::handleVocabLoadSucceeded()
{
    m_logger.verbose(logTag(), QStringLiteral("Vocab load succeeded!"));
    m_logger.verbose(logTag(), QStringLiteral("Emitting success signal..."));
    emit vocabLoadSucceeded();
}

void VocabTrainer::handleVocabLoadSucceededPartially(int failedEntryCount)
{
    m_logger.verbose(logTag(), QStringLiteral("Vocab load succeeded partially. Skipped %1 faulty entries.").arg(QString::number(failedEntryCount)));
    m_logger.verbose(logTag(), QStringLiteral("Emitting partial success signal..."));
    emit vocabLoadSucceededPartially(failedEntryCount);
}

void VocabTrainer::handleVocabLoadFailed(const ErrorCode code)
{
    m_logger.verbose(logTag(), QStringLiteral("Vocab load failed! ErrorCode: %1").arg(errorCodeToString(code)));
    m_logger.verbose(logTag(), QStringLiteral("Emitting failure signal..."));
    emit vocabLoadFailed(code);
}

void VocabTrainer::handleVocabLoadMultipleRequest(const QStringList &filePaths)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling multiple vocab load request..."));
    m_dataService->loadVocabDataMultiple(filePaths);
}

void VocabTrainer::handleVocabLoadMultipleSucceeded()
{
    m_logger.verbose(logTag(), QStringLiteral("Multiple vocab load succeeded!"));
    m_logger.verbose(logTag(), QStringLiteral("Emitting success signal..."));
    emit vocabLoadMultipleSucceeded();
}

void VocabTrainer::handleVocabLoadMultipleSucceededPartially(int failedVocabCount, int partiallySucceededVocabCount, int failedEntryCount)
{
    m_logger.verbose(logTag(), QStringLiteral("Multiple vocab load succeeded partially with %1 failed vocabs and %2 skipped faulty entries in %3 vocabs.")
                                   .arg(QString::number(failedVocabCount), QString::number(failedEntryCount), QString::number(partiallySucceededVocabCount)));
    m_logger.verbose(logTag(), QStringLiteral("Emitting partial success signal..."));
    emit vocabLoadMultipleSucceededPartially(failedVocabCount, partiallySucceededVocabCount, failedEntryCount);
}

void VocabTrainer::handleVocabLoadMultipleFailed(const ErrorCode code)
{
    m_logger.error(logTag(), QStringLiteral("Multiple vocab load failed! ErrorCode: %1").arg(errorCodeToString(code)));
    m_logger.verbose(logTag(), QStringLiteral("Emitting failure signal..."));
    emit vocabLoadMultipleFailed(code);
}

void VocabTrainer::shutdown()
{
    m_logger.verbose(logTag(), QStringLiteral("Shutting down main application..."));
    m_logger.verbose(logTag(),QStringLiteral("Shutting down main application done!"));
}

void VocabTrainer::initializeLogService()
{
    m_logger.verbose(logTag(), QStringLiteral("Initializing log service..."));
    m_logService = new LogService(m_logger, this);
    connect(m_logService, &LogService::queryLogSucceeded, this, &VocabTrainer::handleLogQuerySucceeded);
    connect(m_logService, &LogService::queryLogSucceededPartially, this, &VocabTrainer::handleLogQuerySucceededPartially);
    connect(m_logService, &LogService::queryLogFailed, this, &VocabTrainer::handleLogQueryFailed);
    connect(m_logService, &LogService::requestGetAllLogEntries, &m_logFileHandler, &LogFileHandler::readAllLogEntries);
    connect(&m_logFileHandler, &LogFileHandler::readAllLogEntriesSucceeded, m_logService, &LogService::handleGetAllLogEntriesSucceeded);
    connect(&m_logFileHandler, &LogFileHandler::readAllLogEntriesSucceededPartially, m_logService, &LogService::handleGetAllLogEntriesSucceededPartially);
    m_logger.verbose(logTag(), QStringLiteral("Initializing log service done!"));
}

void VocabTrainer::initializeVocabFileHandler()
{
    m_logger.verbose(logTag(), QStringLiteral("Initializing vocab file handler..."));
    m_vocabFileHandler = new VocabFileHandler(m_logger, this);
    m_logger.verbose(logTag(), QStringLiteral("Initializing vocab file handler done!"));
}

void VocabTrainer::initializeDataService()
{
    m_logger.verbose(logTag(), QStringLiteral("Initializing data service..."));
    m_dataService = new DataService(m_logger, this);

    // Single vocab load
    connect(m_dataService, &DataService::requestVocabFileRead, m_vocabFileHandler, &VocabFileHandler::read);
    connect(m_vocabFileHandler, &VocabFileHandler::readSucceeded, m_dataService, &DataService::vocabFileReadSucceeded);
    connect(m_vocabFileHandler, &VocabFileHandler::readSucceededPartially, m_dataService, &DataService::vocabFileReadSucceededPartially);
    connect(m_vocabFileHandler, &VocabFileHandler::readFailed, m_dataService, &DataService::vocabFileReadFailed);
    connect(m_dataService, &DataService::loadVocabDataSucceeded, this, &VocabTrainer::handleVocabLoadSucceeded);
    connect(m_dataService, &DataService::loadVocabDataSucceededPartially, this, &VocabTrainer::handleVocabLoadSucceededPartially);
    connect(m_dataService, &DataService::loadVocabDataFailed, this, &VocabTrainer::handleVocabLoadFailed);

    // Multiple vocab load
    connect(m_dataService, &DataService::requestVocabFileReadMultiple, m_vocabFileHandler, &VocabFileHandler::readMultiple);
    connect(m_vocabFileHandler, &VocabFileHandler::readMultipleSucceeded, m_dataService, &DataService::vocabFileReadMultipleSucceeded);
    connect(m_vocabFileHandler, &VocabFileHandler::readMultipleSucceededPartially, m_dataService, &DataService::vocabFileReadMultipleSucceededPartially);
    connect(m_vocabFileHandler, &VocabFileHandler::readMultipleFailed, m_dataService, &DataService::vocabFileReadMultipleFailed);
    connect(m_dataService, &DataService::loadVocabDataMultipleSucceeded, this, &VocabTrainer::handleVocabLoadMultipleSucceeded);
    connect(m_dataService, &DataService::loadVocabDataMultipleSucceededPartially, this, &VocabTrainer::handleVocabLoadMultipleSucceededPartially);
    connect(m_dataService, &DataService::loadVocabDataMultipleFailed, this, &VocabTrainer::handleVocabLoadMultipleFailed);

    m_logger.verbose(logTag(), QStringLiteral("Initializing data service done!"));
}
