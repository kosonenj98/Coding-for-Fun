#include "Logging/logfileHandler.h"
#include "Logging/logger.h"
#include "vocabtrainer.h"
#include "GUI/mainwindow.h"

#include <QApplication>
#include <QThread>
#include <QStandardPaths>
#include <QDir>

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("Main");
        return tag;
    }
}

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
    LogFileHandler *logFileHandler = new LogFileHandler(logFilePath);
    logFileHandler->moveToThread(&logThread);
    QObject::connect(&logThread, &QThread::started, logFileHandler, &LogFileHandler::initialize);
    QObject::connect(&logThread, &QThread::finished, logFileHandler, &QObject::deleteLater);

    Logger logger;
    QObject::connect(&logger, &Logger::logEntryCreated, logFileHandler, &LogFileHandler::writeEntry, Qt::QueuedConnection);

    logThread.start();

    logger.info(logTag(), QStringLiteral("Starting VocabTrainer application..."));

    logger.verbose(logTag(), QStringLiteral("Initializing VocabTrainer..."));
    QThread trainerThread;
    trainerThread.setObjectName(QStringLiteral("VocabTrainerThread"));

    VocabTrainer *vocabTrainer = new VocabTrainer(logger, *logFileHandler);
    vocabTrainer->moveToThread(&trainerThread);

    QObject::connect(&trainerThread, &QThread::started, vocabTrainer, &VocabTrainer::initialize);
    QObject::connect(&trainerThread, &QThread::finished, vocabTrainer, &QObject::deleteLater);

    trainerThread.start();
    logger.verbose(logTag(), QStringLiteral("Initializing VocabTrainer initiated..."));

    logger.verbose(logTag(), QStringLiteral("Initializing MainWindow..."));
    MainWindow mainWindow(logger);

    QObject::connect(&mainWindow, &MainWindow::logQueryRequested, vocabTrainer, &VocabTrainer::executeLogQuery, Qt::QueuedConnection);
    QObject::connect(vocabTrainer, &VocabTrainer::logQueryResponded, &mainWindow, &MainWindow::logQueryResponded, Qt::QueuedConnection);

    {
        QString threadId = QString::number(reinterpret_cast<quintptr>(QThread::currentThread()->currentThreadId()), 16);
        QString threadName = QThread::currentThread()->objectName();
        logger.verbose(logTag(), QStringLiteral("Initializing MainWindow done! Running on thread '0x%1' (%2).").arg(threadId, threadName));
    }

    logger.verbose(logTag(), "Showing main window...");
    mainWindow.show();
    logger.verbose(logTag(), "Showing main window done!");

    const int result = a.exec();

    logger.info(logTag(), QStringLiteral("Exiting VocabTrainer application..."));

    QMetaObject::invokeMethod(vocabTrainer, &VocabTrainer::shutdown, Qt::BlockingQueuedConnection);
    trainerThread.quit();
    trainerThread.wait();

    QMetaObject::invokeMethod(logFileHandler, &LogFileHandler::shutdown, Qt::BlockingQueuedConnection);
    logThread.quit();
    logThread.wait();

    return result;
}
