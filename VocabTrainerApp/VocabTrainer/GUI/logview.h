#ifndef LOGVIEW_H
#define LOGVIEW_H

#include "../Logging/logger.h"
#include "../settingshandler.h"

#include <QWidget>
#include <QListWidget>

class LogView : public QWidget
{
    Q_OBJECT
public:
    explicit LogView(Logger &logger, const Settings &settings, QWidget *parent = nullptr);

    void createRequestForLogQueryAndEmit();

    void updateView(const QList<LogEntry> &entries);

public slots:

signals:
    void requestLogQuery(const LogQuery &query);

private:
    Logger &m_logger;
    const QString &m_logFilePath;

    QListWidget *m_logList;
};

#endif // LOGVIEW_H
