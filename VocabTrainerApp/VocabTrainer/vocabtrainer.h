#ifndef VOCABTRAINER_H
#define VOCABTRAINER_H

#include "Logging/logger.h"
#include "logservice.h"
#include "errorcode.h"

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
    void handleLogQuerySucceededPartially(const QList<LogEntry> entries, int failedEntriesCount);
    void handleLogQueryFailed(ErrorCode code);

    void shutdown();

signals:
    void logQuerySucceeded(const QList<LogEntry> entries);
    void logQuerySucceededPartially(const QList<LogEntry> entries, int failedEntriesCount);
    void logQueryFailed(ErrorCode code);

private:
    Logger &m_logger;
    LogFileHandler &m_logFileHandler;
    LogService *m_logService = nullptr;
};
#endif // VOCABTRAINER_H
