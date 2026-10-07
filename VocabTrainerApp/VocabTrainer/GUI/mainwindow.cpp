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

void MainWindow::handleLogQuerySucceeded(const QList<Log::Entry> &entries)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling successful log query..."));
    m_logView->updateView(entries);
    m_logger.verbose(logTag(), QStringLiteral("Handling successful log query done!"));
}

void MainWindow::handleLogQuerySucceededPartially(const QList<Log::Entry> &entries, int failedEntryCount)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling partially successful log query with %1 skipped entries...").arg(failedEntryCount));
    m_logView->updateView(entries);

    m_logger.verbose(logTag(), QStringLiteral("Informing user that log query succeeded partially..."));
    m_dialogService->showWarning(QStringLiteral("Log Query"), QStringLiteral("Skipped %1 faulty entries in selected log file.").arg(QString::number(failedEntryCount)));
    m_logger.verbose(logTag(), QStringLiteral("Informing user that log query succeeded partially done!"));
}

void MainWindow::handleLogQueryFailed(ErrorCode code)
{
    m_logger.verbose(logTag(), QStringLiteral("Informing user that log query failed..."));
    m_dialogService->showError(QStringLiteral("Log Query"), QStringLiteral("Log query failed: (%1)").arg(errorCodeToString(code)));
    m_logger.verbose(logTag(), QStringLiteral("Informing user that log query failed done!"));
}

void MainWindow::handleLogQueryRequest(const Log::Query &query)
{
    m_logger.verbose(logTag(), QStringLiteral("Emitting log query request..."));
    emit requestLogQuery(query);
}

void MainWindow::handleApplyNewSettingsRequest(const Settings &newSettings)
{
    m_logger.verbose(logTag(), QStringLiteral("Emitting request to apply the new settings..."));
    emit requestApplyNewSettings(newSettings);
}

void MainWindow::handleApplyNewSettingsSucceeded()
{
    m_logger.verbose(logTag(), QStringLiteral("Applying the new settings succeeded!"));

    m_logger.verbose(logTag(), QStringLiteral("Informing user about successful settings change..."));
    m_dialogService->showInformation(QStringLiteral("Settings change"), QStringLiteral("Settings saved successfully."));
    m_logger.verbose(logTag(), QStringLiteral("Informing user about successful settings change done!"));
}

void MainWindow::handleApplyNewSettingsFailed(ErrorCode code, const Settings &settings)
{
    m_logger.verbose(logTag(), QStringLiteral("Applying the new settings failed! Reason: %1").arg(errorCodeToString(code)));

    m_logger.verbose(logTag(), QStringLiteral("Informing user that applying the new settings failed..."));
    m_dialogService->showError(QStringLiteral("Settings change"), QStringLiteral("Applying new settings failed: (%1)").arg(errorCodeToString(code)));
    m_logger.verbose(logTag(), QStringLiteral("Informing user that applying the new settings failed done!"));

    m_logger.verbose(logTag(), QStringLiteral("Fixing problematic settings in SettingsView..."));
    m_settingsView->updateView(settings);
    m_logger.verbose(logTag(), QStringLiteral("Fixing problematic settings in SettingsView done!"));
}

void MainWindow::handleViewTabChanged(int index)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling viewTabChanged..."));
    if (index == m_tabWidget->indexOf(m_logView)) {
        m_logView->createInitialView();
    }
    m_logger.verbose(logTag(), QStringLiteral("Handling viewTabChanged done!"));
}
