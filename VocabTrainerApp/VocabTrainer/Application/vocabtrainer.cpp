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

void VocabTrainer::handleLogQueryRequest(const LogQueryRequest &request)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling log query request..."));
    m_logService->queryLog(request);
}

void VocabTrainer::handleLogQueryFinished(const LogQueryResult &result)
{
    emit logQueryFinished(result);
}

void VocabTrainer::handleLoadVocabRequest(const LoadVocabRequest &request)
{
    m_dataService->loadVocab(request);
}

void VocabTrainer::handleLoadVocabInfoFinished(const LoadVocabInfoResult &result)
{
    emit loadVocabInfoFinished(result);
}

void VocabTrainer::handleLoadVocabDataFinished(const LoadVocabDataResult &result)
{
    emit loadVocabDataFinished(result);
}

void VocabTrainer::handleLoadVocabsRequest(const LoadVocabsRequest &request)
{
    m_dataService->loadVocabs(request);
}

void VocabTrainer::handleLoadVocabInfosFinished(const LoadVocabInfosResult &result)
{
    emit loadVocabsInfosFinished(result);
}

void VocabTrainer::handleLoadVocabDatasFinished(const LoadVocabDatasResult &result)
{
    emit loadVocabDatasFinished(result);
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
    connect(m_logService, &LogService::queryLogFinished, this, &VocabTrainer::handleLogQueryFinished);
    connect(m_logService, &LogService::requestGetAllLogEntries, &m_logFileHandler, &LogFileHandler::handleGetAllLogEntriesRequest);
    connect(&m_logFileHandler, &LogFileHandler::getAllLogEntriesFinished, m_logService, &LogService::handleGetAllLogEntriesFinished);
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

    // Single load
    connect(m_dataService, &DataService::readVocabFile, m_vocabFileHandler, &VocabFileHandler::read);
    connect(m_vocabFileHandler, &VocabFileHandler::readInfoFinished, m_dataService, &DataService::handleVocabFileReadInfoFinished);
    connect(m_vocabFileHandler, &VocabFileHandler::readDataFinished, m_dataService, &DataService::handleVocabFileReadDataFinished);

    // Multiple load
    connect(m_dataService, &DataService::readVocabFiles, m_vocabFileHandler, &VocabFileHandler::readMultiple);
    connect(m_vocabFileHandler, &VocabFileHandler::readInfosFinished, m_dataService, &DataService::handleVocabFileReadInfosFinished);
    connect(m_vocabFileHandler, &VocabFileHandler::readDatasFinished, m_dataService, &DataService::handleVocabFileReadDatasFinished);

    m_logger.verbose(logTag(), QStringLiteral("Initializing data service done!"));
}
