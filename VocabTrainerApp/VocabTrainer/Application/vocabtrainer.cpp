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
    initializeTrainingHandler();

    m_logger.verbose(logTag(), QStringLiteral("Initializing main application done!"));

    QString threadId = QString::number(reinterpret_cast<quintptr>(QThread::currentThread()->currentThreadId()), 16);
    QString threadName = QThread::currentThread()->objectName();
    m_logger.verbose(logTag(), QStringLiteral("Initializing VocabTrainer done! Running on thread '0x%1' (%2).").arg(threadId, threadName));
}

void VocabTrainer::handleLogQueryRequest(const Log::QueryRequest &request)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling log query request..."));
    m_logService->queryLog(request);
}

void VocabTrainer::handleLogQueryFinished(const Log::QueryResult &result)
{
    m_logger.debug(logTag(), QStringLiteral("Log query finished!"));
    emit logQueryFinished(result);
}

void VocabTrainer::handleLoadVocabRequest(const Vocab::LoadRequest &request)
{
    m_logger.debug(logTag(), QStringLiteral("Handling load vocab request..."));
    m_dataService->loadVocab(request);
}

void VocabTrainer::handleLoadVocabFinished(const Vocab::LoadResult &result)
{
    m_logger.debug(logTag(), QStringLiteral("Load vocab done!"));
    emit loadVocabFinished(result);
}

void VocabTrainer::handleClearVocabRequest(const Vocab::ClearRequest &request)
{
    m_logger.debug(logTag(), QStringLiteral("Handling vocab clear request..."));
    m_dataService->clearVocab(request);
}

void VocabTrainer::handleClearVocabFinished(const Vocab::ClearResult &result)
{
    m_logger.debug(logTag(), QStringLiteral("Vocab clear request done!"));
    emit clearVocabFinished(result);
}

void VocabTrainer::handleStartTraining(const Training::StartRequest &request)
{
    m_trainingHandler->startTraining(request);
}

void VocabTrainer::handleQuestion(const Training::Question &question)
{
    emit VocabTrainer::question(question);
}

void VocabTrainer::handleAnswer(const Training::Answer &answerData)
{
    m_trainingHandler->handleAnswer(answerData);
}

void VocabTrainer::handleChangeQuestion(const Training::ChangeQuestionRequest &request)
{
    m_trainingHandler->changeQuestion(request);
}

void VocabTrainer::handleChangeQuestionFinished(const Training::ChangeQuestionResult &result)
{
    emit changeQuestionFinished(result);
}

void VocabTrainer::handleNextQuestionAvailable(bool available)
{
    emit nextQuestionAvailable(available);
}

void VocabTrainer::handleTrainingFinished(const Training::Result &result)
{
    emit trainingFinished(result);
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
    connect(m_dataService, &DataService::requestVocabFileRead, m_vocabFileHandler, &VocabFileHandler::handleReadRequest);
    connect(m_vocabFileHandler, &VocabFileHandler::readFinished, m_dataService, &DataService::handleVocabFileReadFinished);
    connect(m_dataService, &DataService::loadVocabFinished, this, &VocabTrainer::handleLoadVocabFinished);
    connect(m_dataService, &DataService::clearVocabFinished, this, &VocabTrainer::handleClearVocabFinished);
    m_logger.verbose(logTag(), QStringLiteral("Initializing data service done!"));
}

void VocabTrainer::initializeTrainingHandler()
{
    m_logger.verbose(logTag(), QStringLiteral("Initializing training handler..."));
    m_trainingHandler = new TrainingHandler(m_logger, this);
    connect(m_trainingHandler, &TrainingHandler::requestGetData, m_dataService, &DataService::handleGetDataRequest);
    connect(m_dataService, &DataService::getDataFinished, m_trainingHandler, &TrainingHandler::handleGetDataFinished);
    connect(m_trainingHandler, &TrainingHandler::question, this, &VocabTrainer::handleQuestion);
    connect(m_trainingHandler, &TrainingHandler::changeQuestionFinished, this, &VocabTrainer::handleChangeQuestionFinished);
    connect(m_trainingHandler, &TrainingHandler::nextQuestionAvailable, this, &VocabTrainer::handleNextQuestionAvailable);
    connect(m_trainingHandler, &TrainingHandler::trainingFinished, this, &VocabTrainer::handleTrainingFinished);
    m_logger.verbose(logTag(), QStringLiteral("Initializing training handler done!"));
}
