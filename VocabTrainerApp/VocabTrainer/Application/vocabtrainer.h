#ifndef VOCABTRAINER_H
#define VOCABTRAINER_H

#include "Logging/logger.h"
#include "logservice.h"
#include "errorcode.h"
#include "vocabfilehandler.h"

#include <QObject>

class VocabTrainer : public QObject
{
    Q_OBJECT

public:
    VocabTrainer(Logger &logger, LogFileHandler &handler, QObject *parent = nullptr);
    ~VocabTrainer();

public slots:
    void initialize();

    void handleLogQueryRequest(const LogQuery &query);
    void handleLogQuerySucceeded(const QList<LogEntry> entries);
    void handleLogQuerySucceededPartially(const QList<LogEntry> entries, int failedEntryCount);
    void handleLogQueryFailed(const ErrorCode code);

    void handleVocabFileReadRequest(const QString &filePath);
    void handleVocabFileReadSucceeded(const VocabFileData &data);
    void handleVocabFileReadSucceededPartially(const VocabFileData &data, int failedEntryCount);
    void handleVocabFileReadFailed(const ErrorCode code);

    void shutdown();

signals:
    void logQuerySucceeded(const QList<LogEntry> entries);
    void logQuerySucceededPartially(const QList<LogEntry> entries, int failedEntryCount);
    void logQueryFailed(const ErrorCode code);

    void vocabFileReadSucceeded(const VocabFileData &data);
    void vocabFileReadSucceededPartially(const VocabFileData &data, int failedEntryCount);
    void vocabFileReadFailed(const ErrorCode code);

private:
    Logger &m_logger;
    LogFileHandler &m_logFileHandler;
    LogService *m_logService = nullptr;
    VocabFileHandler *m_vocabFileHandler = nullptr;
};
#endif // VOCABTRAINER_H
