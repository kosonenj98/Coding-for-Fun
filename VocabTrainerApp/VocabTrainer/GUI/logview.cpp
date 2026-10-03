#include "logview.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QStandardPaths>
#include <QDir>

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("LogView");
        return tag;
    }
}

LogView::LogView(Logger &logger, QWidget *parent)
    : QWidget{parent}, m_logger(logger)
{
    m_logger.verbose(logTag(), QStringLiteral("Constructing log view..."));

    m_logList = new QListWidget(this);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_logList);

    m_logger.verbose(logTag(), QStringLiteral("Constructing log view done!"));
}

void LogView::requestInitialLog()
{
    m_logger.verbose(logTag(), QStringLiteral("Requesting initial dialog..."));
    LogQuery query;

    const QString directory =
        QStandardPaths::writableLocation(
            QStandardPaths::AppLocalDataLocation);

    query.filePath =
        QDir(directory).filePath(
            QStringLiteral("vocabtrainer.log"));

    m_logger.verbose(logTag(), QStringLiteral("Requesting log refresh..."));
    emit logRefreshRequested(query);
}

void LogView::logRefreshResponded(const QList<LogEntry> &entries)
{
    m_logger.verbose(logTag(), QStringLiteral("Requesting log refresh done!"));
    showLogEntries(entries);
}

void LogView::showLogEntries(const QList<LogEntry> &entries)
{
    m_logger.verbose(logTag(), QStringLiteral("Showing log entries..."));
    m_logList->clear();

    for (const LogEntry& entry : entries) {
        const QString threadId = QString::number(reinterpret_cast<quintptr>(entry.threadId), 16);
        const QString text =
            QStringLiteral("[%1] | [%2] | [%3] | [%4] | [%5] | %6: %7")
                .arg(entry.timestamp.toString(Qt::ISODate))
                .arg(entry.sequence)
                .arg(threadId)
                .arg(entry.threadName)
                .arg(logLevelToString(entry.level))
                .arg(entry.tag)
                .arg(entry.message);

        m_logList->addItem(text);
    }

    m_logger.verbose(logTag(), QStringLiteral("Showing log entries done!"));
}
