#include "mainwindow.h"

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("MainWindow");
        return tag;
    }
}

MainWindow::MainWindow(Logger &logger, SettingsHandler &handler, QWidget *parent)
    : QMainWindow{parent}, m_logger(logger), m_settingsHandler(handler)
{
    m_logger.verbose(logTag(), QStringLiteral("Constructing main window..."));
    m_tabWidget = new QTabWidget(this);

    setWindowTitle(QStringLiteral("VocabTrainer"));

    setCentralWidget(m_tabWidget);

    m_logger.verbose(logTag(), QStringLiteral("Initializing main view..."));
    m_mainView = new MainView(logger, this);
    m_logger.verbose(logTag(), QStringLiteral("Initializing main view done!"));

    m_logger.verbose(logTag(), QStringLiteral("Initializing settings view..."));
    m_settingsView = new SettingsView(m_logger, m_settingsHandler.settings(), this);
    connect(m_settingsView, &::SettingsView::requestApplyNewSettings, this, &MainWindow::handleApplyNewSettingsRequest);
    m_logger.verbose(logTag(), QStringLiteral("Initializing settings view done!"));

    m_logger.verbose(logTag(), QStringLiteral("Initializing log view..."));
    m_logView = new LogView(logger, m_settingsHandler.settings(), this);
    connect(m_logView, &LogView::requestLogQuery, this, &MainWindow::handleLogQueryRequest);
    m_logger.verbose(logTag(), QStringLiteral("Initializing log view done!"));

    m_logger.verbose(logTag(), QStringLiteral("Initializing tab widget..."));
    m_tabWidget->addTab(m_mainView, QStringLiteral("Main View"));
    m_tabWidget->addTab(m_settingsView, QStringLiteral("Settings View"));
    m_tabWidget->addTab(m_logView, QStringLiteral("Log View"));
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::handleViewTabChanged);
    m_logger.verbose(logTag(), QStringLiteral("Initializing tab widget done!"));

    m_logger.verbose(logTag(), QStringLiteral("Initializing dialog service..."));
    m_dialogService = new DialogService(this, this);
    m_logger.verbose(logTag(), QStringLiteral("Initializing dialog service done!"));

    m_logger.verbose(logTag(), QStringLiteral("Constructing main window done!"));
}

void MainWindow::handleLogQueryRequest(const LogQueryRequest &request)
{
    m_logger.verbose(logTag(), QStringLiteral("Emitting log query request..."));
    emit requestLogQuery(request);
}

void MainWindow::handleLogQueryFinished(const LogQueryResult &result)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling log query finished..."));
    if (result.code != ErrorCode::Success)
    {
        m_logger.verbose(logTag(), QStringLiteral("Informing user about failed log query..."));
        m_dialogService->showError(QStringLiteral("Log Query"), QStringLiteral("Log query failed: (%1)").arg(errorCodeToString(result.code)));
        m_logger.verbose(logTag(), QStringLiteral("Informing user about failed log query done!"));
        return;
    }

    m_logger.verbose(logTag(), QStringLiteral("Updating log view..."));
    m_logView->updateView(result.entries);
    m_logger.verbose(logTag(), QStringLiteral("Updating log view done!"));

    if (result.failedEntryCount > 0)
    {
        m_dialogService->showWarning(QStringLiteral("Log Query"), QStringLiteral("Skipped %1 faulty entries in selected log file.").arg(QString::number(result.failedEntryCount)));
    }

    m_logger.verbose(logTag(), QStringLiteral("Handling log query finished done!"));
}

void MainWindow::handleApplyNewSettingsRequest(const ApplyNewSettingsRequest &request)
{
    m_logger.verbose(logTag(), QStringLiteral("Emitting request to apply the new settings..."));
    emit requestApplyNewSettings(request);
}

void MainWindow::handleApplyNewSettingsFinished(const ApplyNewSettingsResult &result)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling applying new settings finished..."));
    if (result.code != ErrorCode::Success)
    {
        m_dialogService->showError(QStringLiteral("Settings change"), QStringLiteral("Applying new settings failed: (%1)").arg(errorCodeToString(result.code)));
        m_settingsView->updateView(result.settings);
        return;
    }

    m_dialogService->showInformation(QStringLiteral("Settings change"), QStringLiteral("Settings saved successfully."));
    m_logger.verbose(logTag(), QStringLiteral("Handling applying new settings finished done!"));
}

void MainWindow::handleViewTabChanged(int index)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling viewTabChanged..."));
    if (index == m_tabWidget->indexOf(m_logView)) {
        m_logView->createInitialView();
    }
    m_logger.verbose(logTag(), QStringLiteral("Handling viewTabChanged done!"));
}
