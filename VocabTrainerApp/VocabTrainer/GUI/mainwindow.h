#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../Logging/logger.h"
#include "mainview.h"
#include "logview.h"

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(Logger &logger, QWidget *parent = nullptr);

public slots:
    void logQueryResponded(const QList<LogEntry> &entries);

    void logRefreshRequested(const LogQuery &query);

    void viewTabChanged(int index);

signals:
    void logQueryRequested(const LogQuery &query);

    void logRefreshResponded(const QList<LogEntry> &entries);

private:
    Logger &m_logger;

    QTabWidget *m_tabWidget;

    MainView *m_mainView = nullptr;
    LogView *m_logView = nullptr;
};

#endif // MAINWINDOW_H
