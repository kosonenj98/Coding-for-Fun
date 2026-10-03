#ifndef LOGSERVICE_H
#define LOGSERVICE_H

#include "Logging/logfileHandler.h"

#include <QObject>

class LogService : public QObject
{
    Q_OBJECT
public:
    explicit LogService(Logger &logger, LogFileHandler &handler, QObject *parent = nullptr);

    void executeLogQuery(const LogQuery &query);

public slots:

    void readEntriesResponded(const LogQuery &query, const QList<LogEntry> &entries);

signals:
    void readEntriesRequested(const LogQuery &query);
    void executeLogQueryFinished(const QList<LogEntry> &entries);

private:
    QList<LogEntry> handleLogEntries(const LogQuery &query, const QList<LogEntry> &entries);

    Logger &m_logger;
    LogFileHandler &m_logFileHandler;

};

#endif // LOGSERVICE_H
