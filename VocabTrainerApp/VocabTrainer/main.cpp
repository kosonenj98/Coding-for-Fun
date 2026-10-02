#include "vocabtrainer.h"

#include <QApplication>
#include <QThread>
#include <QStandardPaths>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("VocabTrainer"));

    QThread::currentThread()->setObjectName(QStringLiteral("MainThread"));

    QThread logThread;
    logThread.setObjectName(QStringLiteral("LogWriterThread"));

    const QString logDirectory =QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(logDirectory);
    const QString logFilePath = QDir(logDirectory).filePath(QStringLiteral("vocabtrainer.log"));
    LogFileWriter *logFileWriter = new LogFileWriter(logFilePath);
    logFileWriter->moveToThread(&logThread);
    QObject::connect(&logThread, &QThread::started, logFileWriter, &LogFileWriter::initialize);
    QObject::connect(&logThread, &QThread::finished, logFileWriter, &QObject::deleteLater);

    Logger logger;
    QObject::connect(&logger, &Logger::logEntryCreated, logFileWriter, &LogFileWriter::writeEntry, Qt::QueuedConnection);

    logThread.start();

    VocabTrainer vt(logger);
    vt.initialize();

    const int result = a.exec();

    vt.shutdown();
    QMetaObject::invokeMethod(logFileWriter, &LogFileWriter::shutdown, Qt::BlockingQueuedConnection);
    logThread.quit();
    logThread.wait();

    return result;
}
