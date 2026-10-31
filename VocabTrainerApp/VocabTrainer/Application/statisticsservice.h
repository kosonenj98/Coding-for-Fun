#ifndef STATISTICSSERVICE_H
#define STATISTICSSERVICE_H

#include "Logging/logger.h"
#include "DataTypes/statistics.h"
#include "statisticsanalyzer.h"

#include <QObject>

class StatisticsService : public QObject
{
    Q_OBJECT
public:
    explicit StatisticsService(Logger &logger, QObject *parent = nullptr);

    void handleAnalyzeTrainingRequest(const Statistics::AnalyzeTrainingRequest &request);

signals:
    void analyzeTrainingFinished(const Statistics::AnalyzeTrainingResult &result);

private:
    Logger &m_logger;
    StatisticsAnalyzer m_analyzer;
};

#endif // STATISTICSSERVICE_H
