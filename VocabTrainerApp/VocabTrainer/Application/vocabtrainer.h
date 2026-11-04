#ifndef VOCABTRAINER_H
#define VOCABTRAINER_H

#include "Logging/logger.h"
#include "Logging/logfilehandler.h"
#include "DataTypes/log.h"
#include "logservice.h"
#include "vocabfilehandler.h"
#include "dataservice.h"
#include "traininghandler.h"
#include "statisticsservice.h"

#include <QObject>

class VocabTrainer : public QObject
{
    Q_OBJECT

public:
    VocabTrainer(Logger &logger, LogFileHandler &handler, QObject *parent = nullptr);
    ~VocabTrainer();

public slots:
    void initialize();
    void handleInitializeStatisticsDatabaseFinished(const Statistics::InitializeDatabaseResult &result);

    void handleLogQueryRequest(const Log::QueryRequest &request);
    void handleLogQueryFinished(const Log::QueryResult &result);

    void handleLoadVocabRequest(const Vocab::LoadRequest &request);
    void handleLoadVocabFinished(const Vocab::LoadResult &result);

    void handleClearVocabRequest(const Vocab::ClearRequest &request);
    void handleClearVocabFinished(const Vocab::ClearResult &result);

    void handleStartTrainingRequest(const Training::StartRequest &request);
    void handleQuestion(const Training::Question &question);
    void handleAnswer(const Training::Answer &answerData);
    void handleChangeQuestionRequest(const Training::ChangeQuestionRequest &request);
    void handleChangeQuestionFinished(const Training::ChangeQuestionResult &result);
    void handleNextQuestionAvailable(bool available);
    void handleTrainingFinished(const Training::FinishedResult &result);
    void handleContinueTrainingRequest(const Training::ContinueRequest &request);
    void handleStopTrainingRequest(const Training::StopRequest &request);
    void handleStopTrainingFinished(const Training::StopResult &result);

    void shutdown();

signals:
    void initializeStatisticsDatabaseFinished(const Statistics::InitializeDatabaseResult &result);
    void logQueryFinished(const Log::QueryResult &result);
    void loadVocabFinished(const Vocab::LoadResult &result);
    void clearVocabFinished(const Vocab::ClearResult &result);
    void question(const Training::Question &question);
    void changeQuestionFinished(const Training::ChangeQuestionResult &result);
    void nextQuestionAvailable(bool available);
    void trainingFinished(const Training::FinishedResult &result);
    void stopTrainingFinished(const Training::StopResult &result);

private:
    void initializeLogService();
    void initializeVocabFileHandler();
    void initializeDataService();
    void initializeTrainingHandler();
    void initializeStatisticsService();

    Logger &m_logger;
    LogFileHandler &m_logFileHandler;

    // Created by VocabTrainer
    LogService *m_logService = nullptr;
    VocabFileHandler *m_vocabFileHandler = nullptr;
    DataService *m_dataService = nullptr;
    TrainingHandler *m_trainingHandler = nullptr;
    StatisticsService *m_statisticsService = nullptr;
};
#endif // VOCABTRAINER_H
