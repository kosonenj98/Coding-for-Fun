#ifndef LOGSERVICE_H
#define LOGSERVICE_H

#include "Logging/logfileHandler.h"
#include "errorcode.h"

#include <QObject>

class LogService : public QObject
{
    Q_OBJECT
public:
    explicit LogService(Logger &logger, LogFileHandler &handler, QObject *parent = nullptr);

    void queryLog(const LogQuery &query);

public slots:
    void handleGetAllLogEntriesSucceeded(const LogQuery &query, const QList<LogEntry> &entries);
    void handleGetAllLogEntriesSucceededPartially(const LogQuery &query, const QList<LogEntry> &entries, int failedEntryCount);
    void handleGetAllLogEntriesFailed(ErrorCode code);

signals:
    void requestGetAllLogEntries(const LogQuery &query);
    void queryLogSucceeded(const QList<LogEntry> &entries);
    void queryLogSucceededPartially(const QList<LogEntry> &entries, int failedEntryCount);
    void queryLogFailed(ErrorCode code);

private:
    QList<LogEntry> filterLogEntries(const LogQuery &query, const QList<LogEntry> &entries);

    Logger &m_logger;
    LogFileHandler &m_logFileHandler;

};

#endif // LOGSERVICE_H
