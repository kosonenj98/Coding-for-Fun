#ifndef VOCABTRAINER_H
#define VOCABTRAINER_H

#include "Logging/logger.h"
#include "logservice.h"

#include <QObject>

class VocabTrainer : public QObject
{
    Q_OBJECT

public:
    VocabTrainer(Logger &logger, LogFileHandler &handler, QObject *parent = nullptr);
    ~VocabTrainer();

public slots:
    void initialize();

    void executeLogQuery(const LogQuery &query);

    void executeLogQueryFinished(const QList<LogEntry> entries);

    void shutdown();

signals:
    void logQueryResponded(const QList<LogEntry> &entries);

private:
    Logger &m_logger;
    LogFileHandler &m_logFileHandler;
    LogService *m_logService = nullptr;
};
#endif // VOCABTRAINER_H
