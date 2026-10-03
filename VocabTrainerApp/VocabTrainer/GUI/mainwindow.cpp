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
    connect(m_settingsView, &::SettingsView::settingsChangeRequested, this, &MainWindow::settingsChangeRequested);
    connect(this, &MainWindow::settingsChangeResponded, m_settingsView, &SettingsView::settingsChangeResponded);

    m_logger.verbose(logTag(), QStringLiteral("Initializing settings view done!"));

    m_logger.verbose(logTag(), QStringLiteral("Initializing log view..."));
    m_logView = new LogView(logger, this);
    connect(m_logView, &LogView::logRefreshRequested, this, &MainWindow::logRefreshRequested);
    connect(this, &MainWindow::logRefreshResponded, m_logView, &LogView::logRefreshResponded);
    m_logger.verbose(logTag(), QStringLiteral("Initializing log view done!"));

    m_logger.verbose(logTag(), QStringLiteral("Initializing tab widget..."));
    m_tabWidget->addTab(m_mainView, QStringLiteral("Main View"));
    m_tabWidget->addTab(m_settingsView, QStringLiteral("Settings View"));
    m_tabWidget->addTab(m_logView, QStringLiteral("Log View"));
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::viewTabChanged);
    m_logger.verbose(logTag(), QStringLiteral("Initializing tab widget done!"));

    m_dialogService = new DialogService(this, this);
    connect(this, &MainWindow::settingsChangeSucceeded, m_dialogService, &DialogService::showInformation);

    m_logger.verbose(logTag(), QStringLiteral("Constructing main window done!"));
}

void MainWindow::logQueryResponded(const QList<LogEntry> &entries)
{
    m_logger.verbose(logTag(), QStringLiteral("Requesting log query done!"));
    emit logRefreshResponded(entries);
}

void MainWindow::logRefreshRequested(const LogQuery &query)
{
    m_logger.verbose(logTag(), QStringLiteral("Requesting log query..."));
    emit logQueryRequested(query);
}

void MainWindow::settingsChangeRequested(const Settings &newSettings)
{
    m_logger.verbose(logTag(), QStringLiteral("Requesting settings change..."));
    emit applySettingsRequested(newSettings);
}

void MainWindow::applySettingsResponded(const Settings &settings)
{
    m_logger.verbose(logTag(), QStringLiteral("Requesting settings change done!"));

    m_logger.verbose(logTag(), QStringLiteral("Responding to changing settings request..."));
    emit settingsChangeResponded(settings);

    m_logger.verbose(logTag(), QStringLiteral("Informing user about settings change..."));
    emit settingsChangeSucceeded(QStringLiteral("Settings change"), QStringLiteral("Settings saved successfully."));
}

void MainWindow::viewTabChanged(int index)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling viewTabChanged..."));
    if (index == m_tabWidget->indexOf(m_logView)) {
        m_logView->requestInitialLog();
    }
    m_logger.verbose(logTag(), QStringLiteral("Handling viewTabChanged done!"));
}
