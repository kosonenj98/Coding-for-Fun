#include "vocabtrainer.h"

#include <QThread>
#include <QStandardPaths>
#include <QDir>

namespace
{
    const QString LOG_TAG = QStringLiteral("VocabTrainer");
}

VocabTrainer::VocabTrainer(QObject *parent)
    : QObject(parent)
{
    m_logger.verbose(LOG_TAG, QStringLiteral("Constructing main application..."));
    m_logger.verbose(LOG_TAG, QStringLiteral("Constructing main application done!"));
}

VocabTrainer::~VocabTrainer()
{
    m_logger.verbose(LOG_TAG, QStringLiteral("Destructing main application..."));
    m_logger.verbose(LOG_TAG, QStringLiteral("Destructing main application done!"));
}

void VocabTrainer::initialize()
{
    m_logger.verbose(LOG_TAG, QStringLiteral("Initializing main application..."));

    m_logThread = new QThread(this);
    m_logThread->setObjectName(QStringLiteral("LogWriterThread"));

    const QString logDirectory =QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(logDirectory);
    const QString logFilePath = QDir(logDirectory).filePath(QStringLiteral("vocabtrainer.log"));

    m_logFileWriter = new LogFileWriter(logFilePath);
    connect(m_logThread, &QThread::started, m_logFileWriter, &LogFileWriter::initialize);
    connect(&m_logger, &Logger::logEntryCreated, m_logFileWriter, &LogFileWriter::writeEntry, Qt::QueuedConnection);
    m_logFileWriter->moveToThread(m_logThread);

    m_logThread->start();

    m_logger.debug(LOG_TAG, QStringLiteral("Hello world!"));
    m_logger.verbose(LOG_TAG, QStringLiteral("Initializing main application done!"));
}
