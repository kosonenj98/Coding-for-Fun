#ifndef LOGSERVICE_H
#define LOGSERVICE_H

#include "../Logging/logger.h"
#include "errorcode.h"

#include <QObject>

class LogService : public QObject
{
    Q_OBJECT
public:
    explicit LogService(Logger &logger, QObject *parent = nullptr);

    void queryLog(const Log::Query &query);

public slots:
    void handleGetAllLogEntriesSucceeded(const Log::Query &query, const QList<Log::Entry> &entries);
    void handleGetAllLogEntriesSucceededPartially(const Log::Query &query, const QList<Log::Entry> &entries, int failedEntryCount);
    void handleGetAllLogEntriesFailed(ErrorCode code);

signals:
    void requestGetAllLogEntries(const Log::Query &query);
    void queryLogSucceeded(const QList<Log::Entry> &entries);
    void queryLogSucceededPartially(const QList<Log::Entry> &entries, int failedEntryCount);
    void queryLogFailed(ErrorCode code);

private:
    QList<Log::Entry> filterLogEntries(const Log::Query &query, const QList<Log::Entry> &entries);

    Logger &m_logger;
};

#endif // LOGSERVICE_H
