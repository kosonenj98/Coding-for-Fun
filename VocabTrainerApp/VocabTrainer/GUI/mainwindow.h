#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Logging/logger.h"
#include "Application/settingshandler.h"
#include "DataTypes/requests.h"
#include "DataTypes/results.h"
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
    void handleLogQueryRequest(const LogQueryRequest &request);
    void handleLogQueryFinished(const LogQueryResult &result);

    void handleApplyNewSettingsRequest(const ApplyNewSettingsRequest &request);
    void handleApplyNewSettingsFinished(const ApplyNewSettingsResult &result);

    void handleViewTabChanged(int index);

signals:
    void requestLogQuery(const LogQueryRequest &query);
    void requestApplyNewSettings(const ApplyNewSettingsRequest &request);

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
