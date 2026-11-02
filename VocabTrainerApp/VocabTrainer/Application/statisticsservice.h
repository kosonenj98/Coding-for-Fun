#ifndef STATISTICSSERVICE_H
#define STATISTICSSERVICE_H

#include "Logging/logger.h"
#include "DataTypes/statistics.h"
#include "statisticsanalyzer.h"
#include "statisticsdbhandler.h"

#include <QObject>

class StatisticsService : public QObject
{
    Q_OBJECT
public:
    explicit StatisticsService(Logger &logger, QObject *parent = nullptr);

    void handleInitializeStatisticsDatabaseRequest(const Statistics::InitializeDatabaseRequest &request);
    void handleAnalyzeTrainingRequest(const Statistics::AnalyzeTrainingRequest &request);

signals:
    void initializeDatabaseFinished(const Statistics::InitializeDatabaseResult &result);
    void analyzeTrainingFinished(const Statistics::AnalyzeTrainingResult &result);

private:
    Logger &m_logger;
    StatisticsAnalyzer m_analyzer;
    StatisticsDBHandler m_dbHandler;
};

#endif // STATISTICSSERVICE_H
