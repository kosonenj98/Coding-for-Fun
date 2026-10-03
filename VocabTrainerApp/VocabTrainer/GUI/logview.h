#ifndef LOGVIEW_H
#define LOGVIEW_H

#include "../Logging/logger.h"

#include <QWidget>
#include <QListWidget>

class LogView : public QWidget
{
    Q_OBJECT
public:
    explicit LogView(Logger &logger, QWidget *parent = nullptr);

    void requestInitialLog();

public slots:
    void logRefreshResponded(const QList<LogEntry> &entries);

    void showLogEntries(const QList<LogEntry> &entries);

signals:
    void logRefreshRequested(const LogQuery &query);

private:
    Logger &m_logger;

    QListWidget *m_logList;
};

#endif // LOGVIEW_H
