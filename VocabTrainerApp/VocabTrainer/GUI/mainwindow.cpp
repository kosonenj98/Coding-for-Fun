#include "mainwindow.h"
#include "mainview.h"
#include "logview.h"

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

    MainView *mainView = new MainView(logger, this);
    LogView *logView = new LogView(logger, this);

    m_tabWidget->addTab(mainView, QStringLiteral("Main View"));
    m_tabWidget->addTab(logView, QStringLiteral("Log View"));

    m_logger.verbose(logTag(), QStringLiteral("Constructing main window done!"));
}
