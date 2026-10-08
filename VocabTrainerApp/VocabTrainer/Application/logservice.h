#ifndef LOGSERVICE_H
#define LOGSERVICE_H

#include "Logging/logger.h"
#include "DataTypes/requests.h"
#include "DataTypes/results.h"

#include <QObject>

class LogService : public QObject
{
    Q_OBJECT
public:
    explicit LogService(Logger &logger, QObject *parent = nullptr);

    void queryLog(const LogQueryRequest &request);

public slots:
    void handleGetAllLogEntriesFinished(const GetAllLogEntriesResult &result);

signals:
    void requestGetAllLogEntries(const GetAllLogEntriesRequest &request);

    void queryLogFinished(const LogQueryResult &result);

private:
    QList<Log::Entry> filterLogEntries(const LogQueryRequest &query, const QList<Log::Entry> &entries);

    Logger &m_logger;
};

#endif // LOGSERVICE_H
