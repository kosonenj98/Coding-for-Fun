#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Logging/logger.h"
#include "DataTypes/log.h"
#include "DataTypes/settings.h"
#include "Application/settingshandler.h"
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
    void handleLogQueryRequest(const Log::QueryRequest &request);
    void handleLogQueryFinished(const Log::QueryResult &result);

    void handleApplyNewSettingsRequest(const Settings::ApplyNewRequest &request);
    void handleApplyNewSettingsFinished(const Settings::ApplyNewResult &result);

    void handleLoadVocabRequest(const Vocab::LoadRequest &request);
    void handleLoadVocabFinished(const Vocab::LoadResult &result);

    void handleClearVocabRequest(const Vocab::ClearRequest &request);
    void handleClearVocabFinished(const Vocab::ClearResult &result);

    void handleViewTabChanged(int index);

signals:
    void requestLogQuery(const Log::QueryRequest &query);
    void requestApplyNewSettings(const Settings::ApplyNewRequest &request);
    void requestLoadVocab(const Vocab::LoadRequest &request);
    void requestClearVocab(const Vocab::ClearRequest &request);

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
