#ifndef VOCABTRAINER_H
#define VOCABTRAINER_H

#include "Logging/logger.h"
#include "Logging/logfilehandler.h"
#include "logservice.h"
#include "vocabfilehandler.h"
#include "dataservice.h"
#include "DataTypes/log.h"

#include <QObject>

class VocabTrainer : public QObject
{
    Q_OBJECT

public:
    VocabTrainer(Logger &logger, LogFileHandler &handler, QObject *parent = nullptr);
    ~VocabTrainer();

public slots:
    void initialize();

    void handleLogQueryRequest(const Log::QueryRequest &request);
    void handleLogQueryFinished(const Log::QueryResult &result);

    void handleLoadVocabRequest(const Vocab::LoadRequest &request);
    void handleLoadVocabFinished(const Vocab::LoadResult &result);

    void handleClearVocabRequest(const Vocab::ClearRequest &request);
    void handleClearVocabFinished(const Vocab::ClearResult &result);

    void shutdown();

signals:
    void logQueryFinished(const Log::QueryResult &result);
    void loadVocabFinished(const Vocab::LoadResult &result);
    void clearVocabFinished(const Vocab::ClearResult &result);

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
