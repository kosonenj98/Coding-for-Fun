#ifndef VOCABTRAINER_H
#define VOCABTRAINER_H

#include "Logging/logger.h"
#include "Logging/logfilehandler.h"
#include "DataTypes/requests.h"
#include "DataTypes/results.h"
#include "logservice.h"
#include "vocabfilehandler.h"
#include "dataservice.h"

#include <QObject>

class VocabTrainer : public QObject
{
    Q_OBJECT

public:
    VocabTrainer(Logger &logger, LogFileHandler &handler, QObject *parent = nullptr);
    ~VocabTrainer();

public slots:
    void initialize();

    void handleLogQueryRequest(const LogQueryRequest &request);
    void handleLogQueryFinished(const LogQueryResult &result);

    void handleLoadVocabRequest(const LoadVocabRequest &request);
    void handleLoadVocabInfoFinished(const LoadVocabInfoResult &result);
    void handleLoadVocabDataFinished(const LoadVocabDataResult &result);

    void handleLoadVocabsRequest(const LoadVocabsRequest &request);
    void handleLoadVocabInfosFinished(const LoadVocabInfosResult &result);
    void handleLoadVocabDatasFinished(const LoadVocabDatasResult &result);

    void shutdown();

signals:
    void logQueryFinished(const LogQueryResult &result);

    void loadVocabInfoFinished(const LoadVocabInfoResult &result);
    void loadVocabDataFinished(const LoadVocabDataResult &result);

    void loadVocabsInfosFinished(const LoadVocabInfosResult &result);
    void loadVocabDatasFinished(const LoadVocabDatasResult &result);

private:
    void initializeLogService();
    void initializeVocabFileHandler();
    void initializeDataService();

    Logger &m_logger;
    LogFileHandler &m_logFileHandler;

    // Created by VocabTrainer
    LogService *m_logService = nullptr;
    VocabFileHandler *m_vocabFileHandler = nullptr;
    DataService *m_dataService = nullptr;
};
#endif // VOCABTRAINER_H
