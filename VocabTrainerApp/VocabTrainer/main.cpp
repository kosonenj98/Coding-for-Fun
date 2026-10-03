#include "Logging/logfileHandler.h"
#include "Logging/logger.h"
#include "settingshandler.h"
#include "vocabtrainer.h"
#include "GUI/mainwindow.h"

#include <QApplication>
#include <QThread>


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

    Logger logger;
    SettingsHandler settingsHandler(logger);

    // Initialize LogFileHandler on its separate logging thread
    QThread logThread;
    logThread.setObjectName(QStringLiteral("LogWriterThread"));
    LogFileHandler *logFileHandler = new LogFileHandler(settingsHandler);
    logFileHandler->moveToThread(&logThread);
    QObject::connect(&logThread, &QThread::started, logFileHandler, &LogFileHandler::initialize);
    QObject::connect(&logThread, &QThread::finished, logFileHandler, &QObject::deleteLater);
    QObject::connect(&logger, &Logger::logEntryCreated, logFileHandler, &LogFileHandler::writeEntry, Qt::QueuedConnection);
    logThread.start();

    logger.info(logTag(), QStringLiteral("Starting VocabTrainer application..."));

    // Initialize VocabTrainer
    logger.verbose(logTag(), QStringLiteral("Initializing VocabTrainer..."));
    QThread trainerThread;
    trainerThread.setObjectName(QStringLiteral("VocabTrainerThread"));
    VocabTrainer *vocabTrainer = new VocabTrainer(logger, *logFileHandler);
    vocabTrainer->moveToThread(&trainerThread);
    QObject::connect(&trainerThread, &QThread::started, vocabTrainer, &VocabTrainer::initialize);
    QObject::connect(&trainerThread, &QThread::finished, vocabTrainer, &QObject::deleteLater);
    trainerThread.start();
    logger.verbose(logTag(), QStringLiteral("Initializing VocabTrainer initiated..."));

    // Initialize MainWindow
    logger.verbose(logTag(), QStringLiteral("Initializing MainWindow..."));
    MainWindow mainWindow(logger, settingsHandler);

    // Connect logQuery events
    QObject::connect(&mainWindow, &MainWindow::requestLogQuery, vocabTrainer, &VocabTrainer::handleLogQueryRequest, Qt::QueuedConnection);
    QObject::connect(vocabTrainer, &VocabTrainer::logQuerySucceeded, &mainWindow, &MainWindow::handleLogQuerySucceeded, Qt::QueuedConnection);
    QObject::connect(vocabTrainer, &VocabTrainer::logQuerySucceededPartially, &mainWindow, &MainWindow::handleLogQuerySucceededPartially, Qt::QueuedConnection);
    QObject::connect(vocabTrainer, &VocabTrainer::logQueryFailed, &mainWindow, &MainWindow::handleLogQueryFailed, Qt::QueuedConnection);

    // Connect setting events
    QObject::connect(&mainWindow, &MainWindow::requestApplyNewSettings, &settingsHandler, &SettingsHandler::handleApplyNewSettingsRequest, Qt::QueuedConnection);
    QObject::connect(&settingsHandler, &SettingsHandler::applyNewSettingsSucceeded, &mainWindow, &MainWindow::handleApplyNewSettingsSucceeded, Qt::QueuedConnection);
    QObject::connect(&settingsHandler, &SettingsHandler::applyNewSettingsFailed, &mainWindow, &MainWindow::handleApplyNewSettingsFailed, Qt::QueuedConnection);

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
