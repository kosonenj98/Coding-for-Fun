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

    void handleLoadVocabsRequest(const LoadVocabsRequest &request);
    void handleLoadVocabDatasFinished(const LoadVocabDatasResult &result);

    void handleViewTabChanged(int index);

signals:
    void requestLogQuery(const LogQueryRequest &query);
    void requestApplyNewSettings(const ApplyNewSettingsRequest &request);
    void requestLoadVocabs(const LoadVocabsRequest &request);

private:
    void initializeDialogService();
    void initializeMainView();
    void initializeSettingsView();
    void initializeLogView();
    void initializeTabWidget();

    Logger &m_logger;
    SettingsHandler &m_settingsHandler;
    DialogService* m_dialogService = nullptr;

    // Views
    MainView *m_mainView = nullptr;
    SettingsView *m_settingsView = nullptr;
    LogView *m_logView = nullptr;

    // GUI components
    QTabWidget *m_tabWidget;
};

#endif // MAINWINDOW_H
