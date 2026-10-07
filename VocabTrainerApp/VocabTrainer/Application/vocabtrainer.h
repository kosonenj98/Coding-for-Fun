#ifndef VOCABTRAINER_H
#define VOCABTRAINER_H

#include "Logging/logger.h"
#include "Logging/logfileHandler.h"
#include "errorcode.h"
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

    void handleLogQueryRequest(const Log::Query &query);
    void handleLogQuerySucceeded(const QList<Log::Entry> entries);
    void handleLogQuerySucceededPartially(const QList<Log::Entry> entries, int failedEntryCount);
    void handleLogQueryFailed(const ErrorCode code);

    void handleVocabLoadRequest(const QString &filePath);
    void handleVocabLoadSucceeded();
    void handleVocabLoadSucceededPartially(int failedEntryCount);
    void handleVocabLoadFailed(const ErrorCode code);

    void handleVocabLoadMultipleRequest(const QStringList &filePaths);
    void handleVocabLoadMultipleSucceeded();
    void handleVocabLoadMultipleSucceededPartially(int failedVocabCount, int partiallySucceededVocabCount, int failedEntryCount);
    void handleVocabLoadMultipleFailed(const ErrorCode code);

    void shutdown();

signals:
    void logQuerySucceeded(const QList<Log::Entry> entries);
    void logQuerySucceededPartially(const QList<Log::Entry> entries, int failedEntryCount);
    void logQueryFailed(const ErrorCode code);

    void vocabLoadSucceeded();
    void vocabLoadSucceededPartially(int failedEntryCount);
    void vocabLoadFailed(const ErrorCode code);

    void vocabLoadMultipleSucceeded();
    void vocabLoadMultipleSucceededPartially(int failedVocabCount, int partiallySucceededVocabCount, int failedEntryCount);
    void vocabLoadMultipleFailed(const ErrorCode code);

private:
    void initializeLogService();
    void initializeVocabFileHandler();
    void initializeDataService();

    Logger &m_logger;
    LogFileHandler &m_logFileHandler;
    LogService *m_logService = nullptr;
    VocabFileHandler *m_vocabFileHandler = nullptr;
    DataService *m_dataService = nullptr;
};
#endif // VOCABTRAINER_H
