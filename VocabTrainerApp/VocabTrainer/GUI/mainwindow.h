#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../Logging/logger.h"
#include "../settingshandler.h"
#include "mainview.h"
#include "settingsview.h"
#include "logview.h"
#include "dialogservice.h"

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(Logger &logger, SettingsHandler &handler, QWidget *parent = nullptr);

public slots:
    void logQueryResponded(const QList<LogEntry> &entries);

    void logRefreshRequested(const LogQuery &query);

    void settingsChangeRequested(const Settings &newSettings);

    void applySettingsResponded(const Settings &settings);

    void viewTabChanged(int index);

signals:
    void logQueryRequested(const LogQuery &query);

    void applySettingsRequested(const Settings &newSettings);

    void settingsChangeResponded(const Settings &settings);
    void settingsChangeSucceeded(const QString &title, const QString &message);

    void logRefreshResponded(const QList<LogEntry> &entries);

private:
    Logger &m_logger;
    SettingsHandler &m_settingsHandler;

    QTabWidget *m_tabWidget;

    MainView *m_mainView = nullptr;
    SettingsView *m_settingsView = nullptr;
    LogView *m_logView = nullptr;

    DialogService* m_dialogService;
};

#endif // MAINWINDOW_H
