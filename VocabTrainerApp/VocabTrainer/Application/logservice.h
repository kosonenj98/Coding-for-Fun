#ifndef LOGSERVICE_H
#define LOGSERVICE_H

#include "Logging/logger.h"
#include "DataTypes/log.h"

#include <QObject>

class LogService : public QObject
{
    Q_OBJECT
public:
    explicit LogService(Logger &logger, QObject *parent = nullptr);

    void queryLog(const Log::QueryRequest &request);

public slots:
    void handleGetAllLogEntriesFinished(const Log::GetAllEntriesResult &result);

signals:
    void requestGetAllLogEntries(const Log::GetAllEntriesRequest &request);

    void queryLogFinished(const Log::QueryResult &result);

private:
    QList<Log::Entry> filterLogEntries(const Log::QueryRequest &query, const QList<Log::Entry> &entries);

    Logger &m_logger;
};

#endif // LOGSERVICE_H
