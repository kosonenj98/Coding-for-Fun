#include "logview.h"

#include <QVBoxLayout>
#include <QLabel>

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("LogView");
        return tag;
    }
}

LogView::LogView(Logger &logger, const Settings &settings, QWidget *parent)
    : QWidget{parent}, m_logger(logger), m_logFilePath(settings.logFilePath)
{
    m_logger.verbose(logTag(), QStringLiteral("Constructing log view..."));

    m_logList = new QListWidget(this);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_logList);

    m_logger.verbose(logTag(), QStringLiteral("Constructing log view done!"));
}

void LogView::createRequestForLogQueryAndEmit()
{
    m_logger.verbose(logTag(), QStringLiteral("Requesting initial dialog..."));
    LogQuery query;
    query.filePath = m_logFilePath;
    m_logger.verbose(logTag(), QStringLiteral("Requesting log refresh..."));
    emit requestLogQuery(query);
}

void LogView::updateView(const QList<LogEntry> &entries)
{
    m_logger.verbose(logTag(), QStringLiteral("Displaying log entries..."));
    m_logList->clear();

    for (const LogEntry& entry : entries) {
        const QString threadId = QString::number(reinterpret_cast<quintptr>(entry.threadId), 16);
        const QString text =
            QStringLiteral("[%1] | [%2] | [0x%3] | [%4] | [%5] | %6: %7")
                .arg(entry.timestamp.toString(Qt::ISODate),
                        QString::number(entry.sequence),
                        threadId,
                        entry.threadName,
                        logLevelToString(entry.level),
                        entry.tag,
                        entry.message);

        m_logList->addItem(text);
    }

    m_logger.verbose(logTag(), QStringLiteral("Displaying log entries done!"));
}
