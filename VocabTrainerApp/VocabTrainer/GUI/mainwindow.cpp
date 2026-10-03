#include "mainwindow.h"

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("MainWindow");
        return tag;
    }
}

MainWindow::MainWindow(Logger &logger, QWidget *parent)
    : QMainWindow{parent}, m_logger(logger)
{
    m_logger.verbose(logTag(), QStringLiteral("Constructing main window..."));
    m_tabWidget = new QTabWidget(this);

    setWindowTitle(QStringLiteral("VocabTrainer"));

    setCentralWidget(m_tabWidget);

    m_mainView = new MainView(logger, this);

    m_logView = new LogView(logger, this);
    connect(m_logView, &LogView::logRefreshRequested, this, &MainWindow::logRefreshRequested);
    connect(this, &MainWindow::logRefreshResponded, m_logView, &LogView::logRefreshResponded);

    m_tabWidget->addTab(m_mainView, QStringLiteral("Main View"));
    m_tabWidget->addTab(m_logView, QStringLiteral("Log View"));

    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::viewTabChanged);

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

void MainWindow::viewTabChanged(int index)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling viewTabChanged..."));
    if (index == m_tabWidget->indexOf(m_logView)) {
        m_logView->requestInitialLog();
    }
    m_logger.verbose(logTag(), QStringLiteral("Handling viewTabChanged done!"));
}
